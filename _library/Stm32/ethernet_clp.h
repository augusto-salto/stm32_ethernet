#ifndef ethernet_clp
#define ethernet_clp

#include "main.h"
#include "ethernet_ssl.h"



#define DHCP_SOCKET     0
#define DNS_SOCKET      1
#define HTTP_SOCKET     2

// FUNCOES STANDART PARA FUNCIONAMENTO DO CHIP
void cs_sel(void);
void cs_desel(void);
uint8_t spi_rb(void);
void spi_wb(uint8_t b);
void Callback_IPAssigned(void);
void Callback_IPConflict(void);
void W5500_ReadBuff(uint8_t* buff, uint16_t len);
void W5500_WriteBuff(uint8_t* buff, uint16_t len);


// FUNCAO PARA INICIAR O PROGRAMA
void init_ethernet_estatico(uint8_t ip[4], uint8_t getway[4]);
void init_ethernet_dinamico(void);


// FUNCOES PARA USAR COMO SERVIDOR
void SendTCPMessage(void);		// FUNCAO AUXILIAR PARA USAR COMO SERVIDOR !
void TCPLoopServer(void);						// FUNCAO PARA USAR COMO SERVIDOR !

void requestCliente(void);		// IMPRIME A REQUEST (MENSAGEM DO CLIENTE) !
void consulta_DNS(void);



#endif



