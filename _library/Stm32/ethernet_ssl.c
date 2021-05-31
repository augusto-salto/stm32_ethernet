#include "ethernet_ssl.h"
#include "main.h"
#include "ethernet_clp.h"
#include "SSLInterface.h"
#include "uart_usb.h"



#define ETH_MAX_BUF_SIZE	2048
#define SERVER_PORT			443
#define SOCK_SMTP			0


unsigned char gServer_IP[4] = {35,201,97,85};													//35.201.97.85
int len, rc, server_fd = 0;
unsigned char request_ssl[ETH_MAX_BUF_SIZE];
int retorno = 0;


void init_ssl(){

	
	wiz_tls_context tlsContext;																							// CRIA VARIAVEL CONTEXTO
	
	retorno = wiz_tls_init(&tlsContext,&server_fd);																		// INICIA O CONTEXTO PASSANDO O NUMERO DO SOQUETE
	
	UART_Printf("cheguei aqui! retorno = %d", retorno);
	wiz_tls_connect(&tlsContext, SERVER_PORT, gServer_IP);									// CONECTA AO SERVIDOR
	
	
	/* Read data from SSL channel  */
		len = wiz_tls_read(&tlsContext, request_ssl, ETH_MAX_BUF_SIZE);				// LE UMA POSSIVEL RESPOSTA DO SERVIDOR
	
	
	if(len > 0)																															// QUANDO TERMINAR DE LER, COMECA A ENVIAR
		{
			UART_Printf("%s : %d \r\n",request_ssl, len);
			/*  Write data to the SSL channel  */
			wiz_tls_write(&tlsContext, request_ssl, len);
		}
	
if(wiz_tls_close_notify(&tlsContext) == 0)																// 
		UART_Printf("SSL closed\r\n");

	/*  Free tls context  */
	wiz_tls_deinit(&tlsContext);																						

}


