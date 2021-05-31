#include "ethernet_ssl.h"
#include "main.h"
#include "ethernet_clp.h"
#include "SSLInterface.h"
#include "uart_usb.h"



#define ETH_MAX_BUF_SIZE	2048
#define SERVER_PORT			443
#define SOCK_SMTP			0


unsigned char gServer_IP[4] = {192,168,128,244};
int len, rc, server_fd = 0;
unsigned char ethBuf077[ETH_MAX_BUF_SIZE];

void init_ssl(){


	wiz_tls_context tlsContext;																							// CRIA VARIAVEL CONTEXTO
	
	wiz_tls_init(&tlsContext,&server_fd);																		// INICIA O CONTEXTO PASSANDO O NUMERO DO SOQUETE
	
	
	wiz_tls_connect(&tlsContext, SERVER_PORT, gServer_IP);									// CONECTA AO SERVIDOR
	
	
	/* Read data from SSL channel  */
		len = wiz_tls_read(&tlsContext, ethBuf077, ETH_MAX_BUF_SIZE);
	
	
	if(len > 0)
		{
			UART_Printf("%s : %d \r\n",ethBuf077, len);
			/*  Write data to the SSL channel  */
			wiz_tls_write(&tlsContext, ethBuf077, len);
		}
	
if(wiz_tls_close_notify(&tlsContext) == 0)
		UART_Printf("SSL closed\r\n");

	/*  Free tls context  */
	wiz_tls_deinit(&tlsContext);

}


