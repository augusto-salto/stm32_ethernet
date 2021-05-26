#include	"ethernet_clp.h"
#include	"wizchip_conf.h"
#include	"socket.h"
#include 	"loopback.h"
#include  "string.h"
#include  "uart_usb.h"
#include <stdbool.h>
#include "dhcp.h"
#include "dns.h"




#define ETH_MAX_BUF_SIZE	2048

unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];
unsigned char ethBuf2[ETH_MAX_BUF_SIZE];
unsigned char ethBuf3[ETH_MAX_BUF_SIZE];



uint8_t rcvBuf[20], bufSize[] = {4, 4, 4, 4, 4, 4, 4, 4};
volatile bool ip_assigned = false;
uint8_t dhcp_buffer[1024];
uint8_t dns_buffer[1024];

void cs_sel(void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); //CS LOW
}

void cs_desel(void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); //CS HIGH
}

uint8_t spi_rb(void) {
	uint8_t rbuf;
	HAL_SPI_Receive(&hspi1, &rbuf, 1, 0xFFFFFFFF);
	return rbuf;
}

void spi_wb(uint8_t b) {
	HAL_SPI_Transmit(&hspi1, &b, 1, 0xFFFFFFFF);
}

void W5500_ReadBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Receive(&hspi1, buff, len, HAL_MAX_DELAY);
}

void W5500_WriteBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Transmit(&hspi1, buff, len, HAL_MAX_DELAY);
}


void Callback_IPAssigned(void) {
   UART_Printf("Callback: IP assigned! Leased time: %d sec\r\n", getDHCPLeasetime());
    ip_assigned = true;
}
 
void Callback_IPConflict(void) {
   UART_Printf("Callback: IP conflict!\r\n");
}







void init() {

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
	
	
   UART_Printf("\r\ninit() called!\r\n");

    UART_Printf("Registering W5500 callbacks...\r\n");
    reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
    reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
    reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

		UART_Printf("Calling wizchip_init()...\r\n");
    uint8_t rx_tx_buff_sizes[] =  {2, 2, 2, 2, 2, 2, 2, 2,  // 8 channel tx
                                            2, 2, 2, 2, 2, 2, 2, 2}; // 8 channel rx
		
    wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

    UART_Printf("Calling DHCP_init()...\r\n");
    wiz_NetInfo net_info = {
        .mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
        .dhcp = NETINFO_DHCP
    };
    // set MAC address before using DHCP
    setSHAR(net_info.mac);
    DHCP_init(DHCP_SOCKET, dhcp_buffer);

    UART_Printf("Registering DHCP callbacks...\r\n");
    reg_dhcp_cbfunc(
        Callback_IPAssigned,
        Callback_IPAssigned,
        Callback_IPConflict
    );

		
		UART_Printf("Calling DHCP_run()...\r\n");
    // actually should be called in a loop, e.g. by timer
    uint32_t ctr = 10000;
    while((!ip_assigned) && (ctr > 0)) {
		
			UART_Printf("\r ctr!  %d :(\r\n",ctr );
			HAL_Delay(100);
			DHCP_run();
        ctr--;
    }
    if(!ip_assigned) {
      UART_Printf("\r\nIP was not assigned :(\r\n");
		
		
        return;
    }

		
    getIPfromDHCP(net_info.ip);
    getGWfromDHCP(net_info.gw);
    getSNfromDHCP(net_info.sn);

    uint8_t dns[4];
    getDNSfromDHCP(dns);

  UART_Printf("IP:  %d.%d.%d.%d\r\nGW:  %d.%d.%d.%d\r\nNet: %d.%d.%d.%d\r\nDNS: %d.%d.%d.%d\r\n",
        net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3],
        net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3],
        net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3],
        dns[0], dns[1], dns[2], dns[3]
    );
		
		
HAL_Delay(500); //TODO
		
		
		
   UART_Printf("Calling wizchip_setnetinfo()...\r\n");
    wizchip_setnetinfo(&net_info);
		
HAL_Delay(500); //TODO

  UART_Printf("Calling DNS_init()...\r\n");
    DNS_init(DNS_SOCKET, dns_buffer);

HAL_Delay(500); //TODO

    uint8_t addr[4];
    {
        char domain_name[] = "eax.me";
   UART_Printf("Resolving domain name \"%s\"...\r\n", domain_name);
        int8_t res = DNS_run(dns, (uint8_t*)&domain_name, addr);
			
			HAL_Delay(500); //TODO
			
        if(res != 1) {
      UART_Printf("DNS_run() failed, res = %d", res);
					
					HAL_Delay(500); //TODO
					
            return;
        }
       UART_Printf("Result: %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
    }

 UART_Printf("Creating socket...\r\n");
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
   /* {
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
				
				
    }*/

		
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










//inicia o chip!
void inicia_chip(){

	reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
  reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
	
	wizchip_init(bufSize, bufSize);
	//wizchip_init((uint8_t*)4 ,(uint8_t*)4);
  wiz_NetInfo netInfo = { .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
                          .ip 	= {192, 168, 15, 101},					// IP address
                          .sn 	= {255, 255, 255, 0},					// Subnet mask
                          .gw 	= {192, 168, 15, 1}};					// Gateway address
  wizchip_setnetinfo(&netInfo);
  wizchip_getnetinfo(&netInfo);


		init();
													
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




void SendTCPMessage()
{
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
void TCPLoopServer()
{
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








