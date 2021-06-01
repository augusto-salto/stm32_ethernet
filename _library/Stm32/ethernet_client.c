#include "ethernet_client.h"
#include "ethernet_clp.h"
#include "socket.h"
#include  "uart_usb.h"
#include  "main.h"
#include <stdio.h>
#include <string.h>
//#include "ethernet_ssl.h"

#define HTTP_PORT       1000
#define RECV_BUFFER_LENGTH  2048


#define INICIO		"PATCH /stm32-ethernet-default-rtdb.json HTTP/1.1\r\n"
#define HOST			"HOST: stm32-ethernet-default-rtdb.firebaseio.com\r\n"
#define CONN			"Content-Type: text/plain;charset=UTF-8\r\n"
#define LENGHT			"Content-Length: 20\r\n"
#define JSON			"{\"nome4\":21}\r\n"


static const char FIREBASE[] = INICIO HOST CONN LENGHT JSON;
	
static const uint8_t server_ip[] = {35, 201, 97, 85};
static const uint16_t server_port = 443;
static uint16_t length = 0;
static uint8_t recv_buf[RECV_BUFFER_LENGTH];
static  const char *request1 = 0;

void client_request(char request[]){

	/*request1 = request;
  length = strlen(request);*/
		request1 = FIREBASE;
  length = strlen(FIREBASE);
	
	
	uint8_t err = 2;
	
	UART_Printf("%s", FIREBASE);
	
	UART_Printf("\r\n Cliente iniciou uma request");
	err = socket(HTTP_SOCKET, Sn_MR_TCP, 10888, 0);										// SE HOUVER ALGUM ERRO, RETORNA O CODIGO DO ERRO!
	HAL_Delay(500);		
		if (err != HTTP_SOCKET)																						// TRATATIVA DO ERRO NO SOCKETE
    {
         UART_Printf("\r \n ERRO AO CONFIGURAR O SOCKET! SOCK_ERR: %u\r\n", (uint16_t) err);	
HAL_Delay(500);			
        return;
    }


	
		
	UART_Printf("\nCONECTANDO AO SERVIDOR");	
		
    err = connect(HTTP_SOCKET, (uint8_t*)server_ip, server_port);	// CONECTA AO SERVIDOR 
		HAL_Delay(500);		
			
		
    if (err != SOCK_OK)																						// TRATATIVA DO ERRO NA CONEXAO COM O SERVIDOR
    {
				 UART_Printf("\nERRO AO CONECTAR COM O SERVIDOR! SOCK_ERR: %u\r\n", (uint16_t) err);																								
        return;
    }
	

	UART_Printf("\nENVIANDO PACOTE");	
		
				
		
    length = send(HTTP_SOCKET, (uint8_t*) request1, length);
   
		UART_Printf("\nTamanho de dados enviados sent: %u\r\n", (uint16_t) length);
		
    if (length == 0) return;
		
		
	UART_Printf("\nRECEBENDO A RESPOSTA DO SERVIDOR");	
		

    length = recv(HTTP_SOCKET, recv_buf, RECV_BUFFER_LENGTH);
			HAL_Delay(500);	
    UART_Printf("\nTamanho de dados recebidos  recv: %u\r\n", (uint16_t) length);

    if (length == 0) return;
	HAL_Delay(500);	
    if (length > (RECV_BUFFER_LENGTH - 1)) length = (RECV_BUFFER_LENGTH - 1);
    recv_buf[length] = '\0';
		
			HAL_Delay(500);	
		    UART_Printf("\n### RESPONSE INICIO ###\r\n");
        UART_Printf((const char*) recv_buf);
        UART_Printf("\n### RESPONSE FIM ###\r\n");
				

				UART_Printf("\nDESCONECTANDO DO SERVIDOR!");	
    err = disconnect(HTTP_SOCKET);
    if (err != SOCK_OK)
    {
        UART_Printf("\nERRO AO DESCONECTAR! SOCK_ERR: %u\r\n", (uint16_t) err);
        return;
    }
		
		
				UART_Printf("\nFECHANDO O SOCKET");
    err = close(HTTP_SOCKET);
    if (err != SOCK_OK)
    {
         UART_Printf("\nERRO AO FECHAR O SOCKET! SOCK_ERR: %u\r\n", (uint16_t) err);
        return;
    }
		
		
				
				
		
}



