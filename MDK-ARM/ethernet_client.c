#include "ethernet_client.h"
#include "ethernet_clp.h"
#include "socket.h"
#include  "uart_usb.h"
#include  "main.h"

#define HTTP_PORT       1000
#define RECV_BUFFER_LENGTH  2048
#define REQUISICAO  OLA_MUNDO

static const uint8_t server_ip[] = {192, 168, 15, 18};
static const uint16_t server_port = 5000;
static uint16_t length = 0;
static const char *request = 0;
static uint8_t recv_buf[RECV_BUFFER_LENGTH];
static const char LIGHT_ON[] = "ola mundo!";
static const char *request1 = LIGHT_ON;

void client_request(char* request){

	
	UART_Printf("\r\n Cliente iniciou uma request");
	HAL_Delay(500);		
	
	uint8_t err = 2;
	
	
	// socket(http_socket, Sn_MR_TCP, 10888, 0);
	//err = socket(HTTP_SOCKET, SOCK_STREAM, HTTP_PORT, 0);						// SE HOUVER ALGUM ERRO, RETORNA O CODIGO DO ERRO!
	err = socket(HTTP_SOCKET, Sn_MR_TCP, 10888, 0);	
	HAL_Delay(500);		
		/*if (err != SOCK_OK)																						// TRATATIVA DO ERRO NO SOCKETE
    {
         UART_Printf("\r \n ERRO AO CONFIGURAR O SOCKET! SOCK_ERR: %u\r\n", (uint16_t) err);	
HAL_Delay(500);			
        return;
    }*/


	
		
	UART_Printf("\rCONECTANDO AO SERVIDOR");	
		
    err = connect(HTTP_SOCKET, (uint8_t*)server_ip, server_port);	// CONECTA AO SERVIDOR 
		HAL_Delay(500);		
			
		
    if (err != SOCK_OK)																						// TRATATIVA DO ERRO NA CONEXAO COM O SERVIDOR
    {
				 UART_Printf("\rERRO AO CONECTAR COM O SERVIDOR! SOCK_ERR: %u\r\n", (uint16_t) err);																								
        return;
    }
	

	UART_Printf("\rENVIANDO PACOTE");	
	
    length = send(HTTP_SOCKET, (uint8_t*) request1, length);
   
		UART_Printf("\rTamanho de dados enviados sent: %u\r\n", (uint16_t) length);
		
    if (length == 0) return;
		
		
	UART_Printf("\rRECEBENDO A RESPOSTA DO SERVIDOR");	
		

    length = recv(HTTP_SOCKET, recv_buf, RECV_BUFFER_LENGTH);
			HAL_Delay(500);	
    UART_Printf("\rTamanho de dados recebidos  recv: %u\r\n", (uint16_t) length);

    if (length == 0) return;
	HAL_Delay(500);	
    if (length > (RECV_BUFFER_LENGTH - 1)) length = (RECV_BUFFER_LENGTH - 1);
    recv_buf[length] = '\0';
		
			HAL_Delay(500);	
		    UART_Printf("### RESPONSE INICIO ###\r\n");
        UART_Printf((const char*) recv_buf);
        UART_Printf("\r\n### RESPONSE FIM ###\r\n");
				
				
				
				UART_Printf("DESCONECTANDO DO SERVIDOR!");	
    err = disconnect(HTTP_SOCKET);
    if (err != SOCK_OK)
    {
        UART_Printf("ERRO AO DESCONECTAR! SOCK_ERR: %u\r\n", (uint16_t) err);
        return;
    }
		
		
				UART_Printf("FECHANDO O SOCKET");
    err = close(HTTP_SOCKET);
    if (err != SOCK_OK)
    {
         UART_Printf("ERRO AO FECHAR O SOCKET! SOCK_ERR: %u\r\n", (uint16_t) err);
        return;
    }
		
		
				
				
		
}



