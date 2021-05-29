#include	"ethernet_clp.h"
#include	"wizchip_conf.h"
#include	"socket.h"
#include  "string.h"
#include  "uart_usb.h"
#include <stdbool.h>
#include "dhcp.h"
#include "dns.h"
//#include 	"loopback.h"



#define ETH_MAX_BUF_SIZE	2048



unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];
unsigned char ethBuf2[ETH_MAX_BUF_SIZE];
unsigned char ethBuf3[ETH_MAX_BUF_SIZE];



uint8_t rcvBuf[20], bufSize[] = {4, 4, 4, 4, 4, 4, 4, 4};
volatile bool ip_assigned = false;
uint8_t dhcp_buffer[1024];
uint8_t dns_buffer[1024];
uint8_t dns[4];
uint8_t addr[4];

// FUNCAO CHIP SELECT 

void cs_sel(void) {																			
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); //CS LOW
}

// FUNCAO CHIP DESELECT

void cs_desel(void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); //CS HIGH
}


// FUNCAO SPI READ BYTE
uint8_t spi_rb(void) {
	uint8_t rbuf;
	HAL_SPI_Receive(&hspi1, &rbuf, 1, 0xFFFFFFFF);
	return rbuf;
}


// FUNCAO SPI WRITE BYTE
void spi_wb(uint8_t b) {
	HAL_SPI_Transmit(&hspi1, &b, 1, 0xFFFFFFFF);
}


// FUNCAO W5500 READ BUFFER
void W5500_ReadBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Receive(&hspi1, buff, len, HAL_MAX_DELAY);
}


// FUNCAO W5500 WRITE BUFFER
void W5500_WriteBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Transmit(&hspi1, buff, len, HAL_MAX_DELAY);
}

// FUNCAO IP ATRIBUIDO
void Callback_IPAssigned(void) {
   UART_Printf("IP ATRIBUIDO VIA DHCP! TEMPO DECORRIDO: %d sec\r\n", getDHCPLeasetime());
    ip_assigned = true;
}


// FUNCAO CONFLITO DE IP 
void Callback_IPConflict(void) {
   UART_Printf("CALLBACK IP EM CONFLITO!\r\n");
}

// FUNCAO PARA DEBUG

void Delay_debug(void){

	HAL_Delay(1000);
	UART_Printf("1\n");
	HAL_Delay(1000);
	UART_Printf("2\n");
	HAL_Delay(1000);
	UART_Printf("3\n");
	HAL_Delay(1000);
	UART_Printf("4\n");
	HAL_Delay(1000);
	UART_Printf("5 \n");
	HAL_Delay(100);

}


	

		///// ***** INICIA W5500 COM ATRIBUICAO DE IP VIA DHCP (IP DINAMICO) ***** /////
void init_ethernet_dinamico() {
	

		Delay_debug();
	
		// INICIANDO O CHIP W5500	//
	
		// CONFIGURANDO O CHIP E O CANAL SPI
    reg_wizchip_cs_cbfunc(cs_sel, cs_desel);																	// Informa quais as funcoes para chip select e deselect
    reg_wizchip_spi_cbfunc(spi_rb, spi_wb);																		// Informa as funcoes de leitura e escrita de byte via barramento SPI
    reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);							// Informa as funcoes de escrita e leitura de buffers do W5500
    uint8_t rx_tx_buff_sizes[] =  {2, 2, 2, 2, 2, 2, 2, 2,  									// Define o tamanho do buffer de escrita (8 canais tx)
                                            2, 2, 2, 2, 2, 2, 2, 2}; 					// Define o tamanho do buffer de leitura (8 canais rx) 
		wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);													// Informa o tamanho dos buffers de escrita e leitura parea o chip W5500

																						
																						
		// CONFIGURANDO O DHCP
    wiz_NetInfo net_info = {																									// Cria uma variavel do tipo struct e informa o endereco mac, 
        .mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },												// Endereco MAC
        .dhcp = NETINFO_DHCP																									// Define se o IP sera dinamico ou estatico. NETINFO_DHCP = Dinamico / NETINFO_STATIC = Estatico
    };
   
    setSHAR(net_info.mac);  																									// Define o endereco MAC local antes de usar o DHCP
    DHCP_init(DHCP_SOCKET, dhcp_buffer);

    reg_dhcp_cbfunc(																													// Definindo as funcoes de retorno do DHCP
        Callback_IPAssigned,																									// Funcao de retorno de chamada quando o IP ¨¦ atribuido a partir do servidor DHCP 
        Callback_IPAssigned,																									// Funcao de retorno de chamada quando o IP ¨¦ alterado
        Callback_IPConflict																										// Funcao de retorno de chamada quando o IP atribu¨ªdo esta em conflito com outros.
    );

		
	
    uint32_t ctr = 10000;																											// Variavel com a quantidade de tentativas para receber um endereco IP. Atentar para nao travar o loop principal aqui. TODO
    while((!ip_assigned) && (ctr > 0)) {																			// 	Loop ate receber um endereco IP
		
			UART_Printf("\r Tentativa de conexao  %d :(\r\n",ctr );
			DHCP_run();																															// Funcao DHCP. Retorno disponivel* TODO
        ctr--;
    }
    if(!ip_assigned) {
      UART_Printf("\r\nIP NAO ATRIBUIDO!(\r\n");
		
		
        return;
    }

		
    getIPfromDHCP(net_info.ip);																								// Armazena o IP na variavel "net_info"
    getGWfromDHCP(net_info.gw);																								// Armazena o Getway na variavel "net_info"
    getSNfromDHCP(net_info.sn);																								// Armazena a Sub-Mascara de rede na variavel "net_info"
		getDNSfromDHCP(dns);																											// Armazena o servidor DNS obtido via DHCP na variavel "net_info"

		
		
		
  UART_Printf("IP:  %d.%d.%d.%d\r\nGW:  %d.%d.%d.%d\r\nNet: %d.%d.%d.%d\r\nDNS: %d.%d.%d.%d\r\n",		// Imprime os enderecos IP, DNS E SUB*REDE obtidos
        net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3],
        net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3],
        net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3],
        dns[0], dns[1], dns[2], dns[3]
    );
		


    wizchip_setnetinfo(&net_info);																						// Envia as informacoes de rede para o W5500
		
 
}






		///// ***** INICIA O W5500 COM ATRIBUICAO DE IP ESTATICO (IP FIXO)***** /////
void init_ethernet_estatico(uint8_t ip[4], uint8_t getway[4]){						// Passa o ip e o getway como parametro
	
	Delay_debug();

	reg_wizchip_cs_cbfunc(cs_sel, cs_desel);																								// Informa quais as funcoes para chip select e deselect
  reg_wizchip_spi_cbfunc(spi_rb, spi_wb);																									// Informa as funcoes de leitura e escrita de byte via barramento SPI
	wizchip_init(bufSize, bufSize);																													// Informa o tamanho dos buffers de escrita e leitura parea o chip W5500
  
	wiz_NetInfo netInfo = {																																	// Cria uma variavel do tipo struct e informa o endereco mac,
													.mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},										// Mac address
                          .ip 	= {ip[0], ip[1], ip[2], ip[3]},														// IP address  --->  .ip 	= {192, 168, 15, 101},	
                          .sn 	= {255, 255, 255, 0},																			// Subnet mask
                          .gw 	= {getway[0], getway[1], getway[2], getway[3]}};					// Gateway address
  wizchip_setnetinfo(&netInfo);
  wizchip_getnetinfo(&netInfo);
		
	UART_Printf("IP:  %d.%d.%d.%d\r\nGW:  %d.%d.%d.%d\r\nNet: %d.%d.%d.%d\r\nDNS: %d.%d.%d.%d\r\n",		// Imprime os enderecos IP, DNS E SUB*REDE obtidos
    netInfo.ip[0], netInfo.ip[1], netInfo.ip[2], netInfo.ip[3],
    netInfo.gw[0], netInfo.gw[1], netInfo.gw[2], netInfo.gw[3],
    netInfo.sn[0], netInfo.sn[1], netInfo.sn[2], netInfo.sn[3],
    dns[0], dns[1], dns[2], dns[3]
    );
													
													
													
}







void requestCliente(){
	
					char buff[32]; 
       
						for(;;) {
            int32_t nbytes = recv(0, (uint8_t*)&buff, sizeof(buff)-1);
							
            if(nbytes == SOCKERR_SOCKSTATUS) {
                UART_Printf("\r\nConnection closed.\r\n");
                break;
            }

            if(nbytes <= 0) {
              UART_Printf("\r\nrecv() failed, %d returned\r\n", nbytes);
                break;
            }

            buff[nbytes] = '\0';
            UART_Printf("%s", buff);
						HAL_Delay(10);
						UART_Printf("\n");
        }		
				

				
}




void SendTCPMessage(){
	
	
	 uint8_t retVal = 0;

	
	  /* While socket is in LISTEN mode we wait for a remote connection */
			  //while(sockStatus = getSn_SR(0) == SOCK_LISTEN)
				//  HAL_Delay(100);
			  /* OK. Got a remote peer. Let's send a message to it */
	 while( getSn_SR(0) == SOCK_LISTEN);
	
	UART_Printf("getSn_SR(0) e %d \n", getSn_SR(0));
	UART_Printf("LISTENING..... \n");
	
	 while(getSn_SR(0) == SOCK_ESTABLISHED) {
			//int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len)
		 
		 
		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
		 
		
		 //UART_Printf(ethBuf0);
				  /* If connection is ESTABLISHED with remote peer */
				  //if(sockStatus = getSn_SR(0) == SOCK_ESTABLISHED) {
					  uint8_t remoteIP[4];
					  uint16_t remotePort;
					  /* Retrieving remote peer IP and port number */
					  getsockopt(0, SO_DESTIP, remoteIP);
					  getsockopt(0, SO_DESTPORT, (uint8_t*)&remotePort);
					  UART_Printf("connection ok from:%d.%d.%d.%d:%d\r\n" ,remoteIP[0],remoteIP[1],remoteIP[2],remoteIP[3], remotePort);

						 /*recebido = recv(0,ethBuf0,sizeof(ethBuf0));
		 
							UART_Printf("recebido... %s \n", recebido );*/
		 
								requestCliente();	// imprime a mensagem do cliente na porta serial!
		
		 
					  /* Let's send a welcome message and closing socket */
					  if(retVal == (int16_t)strlen("*****welcome*****\r\n") || retVal == 0 )
					  {
							
							send(0, "*****welcome*****\r\n", strlen("*****welcome*****\r\n")); 
						  //printf("welcome sent\r\n");
					  }
					  else { /* Ops: something went wrong during data transfer */

						  UART_Printf("something went wrong during data transfer %d\r\n", retVal);
						  break;
					  }
					//
				  }
	 
					
					
					
//				  else { /* Something went wrong with remote peer, maybe the connection was closed unexpectedly */
//					  printf("Something went wrong with remote peer %d\r\n", sockStatus);
//
//					  break;
//				  }
			 // }//end while

}
void TCPLoopServer(){
	
	
	 uint8_t retVal;


	 /* Open socket 0 as TCP_SOCKET with port 5000 */
	  if((retVal = socket(0, Sn_MR_TCP, 5000, 0)) == 0) {
		  /* Put socket in LISTEN mode. This means we are creating a TCP server */
		  if((retVal = listen(0)) == SOCK_OK) {
				
				
				
				
SendTCPMessage();

				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);   		
				
				
				
		  }
		  else /* Ops: socket not in LISTEN mode. Something went wrong */
		 UART_Printf("listen\r\n");
	  } else { /* Can't open the socket. This means something is wrong with W5100 configuration: maybe SPI issue? */
		  UART_Printf("socket open error: %d\r\n", retVal);

	  }

	  /* We close the socket and start a connection again */
	  disconnect(0);
	  close(0);
}






void consulta_DNS(){
	
	
	
	// FUNCAO QUE CONSULTA UM ENDERECO DE UM SITE E RETORNA O IP DELE NA VARIAVEL ADDR
	
char domain_name[] = "eax.me";
	
	
	// INICIANDO O DNS //		
		
    DNS_init(DNS_SOCKET, dns_buffer);																					// Inicia o DNS informando o socket do chip e o tamanho do buffer
		
		UART_Printf("Resolving domain name \"%s\"...\r\n", domain_name);
    int8_t res = DNS_run(dns, (uint8_t*)&domain_name, addr);
	
			if(res != 1) {
      UART_Printf("DNS_run() falhou. Retorno = %d", res);
				return;
        }
			
		UART_Printf("Result: %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);	// Retorna o IP do endereco

}




void acesso_http(){											
 

    uint8_t http_socket = HTTP_SOCKET;			

	 
    uint8_t code = socket(http_socket, Sn_MR_TCP, 10888, 0);
    if(code != http_socket) {
			HAL_Delay(500); //TODO
       UART_Printf("socket() failed, code = %d\r\n", code);
        return;
    }

   UART_Printf("Socket created, connecting...\r\n");
    code = connect(http_socket, addr, 80);
    if(code != SOCK_OK) {
			HAL_Delay(500); //TODO
       UART_Printf("connect() failed, code = %d\r\n", code);
        close(http_socket);
        return;
    }

 UART_Printf("Connected, sending HTTP request...\r\n");
    {
        char req[] = "GET / HTTP/1.1\r\nHost: eax.me\r\n\r\n";
        uint16_t len = sizeof(req) - 1;
        uint8_t* buff = (uint8_t*)&req;
			
			
        while(len > 0) {

				UART_Printf("Sending %d bytes...\r\n", len);

					
				int32_t nbytes = send(http_socket, buff, len);
					
            if(nbytes <= 0) {
       UART_Printf("send() failed, %d returned\r\n", nbytes);
							
                close(http_socket);
                return;
            }
            UART_Printf("%d bytes sent!\r\n", nbytes);
            len -= nbytes;
        }
				
				
    }

		
 UART_Printf("Request sent. Reading response...\r\n");
  /*  {
			
        char buff[32];
        for(;;) {
            int32_t nbytes = recv(http_socket, (uint8_t*)&buff, sizeof(buff)-1);
            if(nbytes == SOCKERR_SOCKSTATUS) {
                UART_Printf("\r\nConnection closed.\r\n");
                break;
            }

            if(nbytes <= 0) {
              UART_Printf("\r\nrecv() failed, %d returned\r\n", nbytes);
                break;
            }

            buff[nbytes] = '\0';
            UART_Printf("%s", buff);
        }
    }*/

    UART_Printf("Closing socket.\r\n");
    close(http_socket);
		
		
 
 
 }






