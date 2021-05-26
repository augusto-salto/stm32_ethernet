#include "uart_usb.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>


// FUNCAO PARA ENVIAR VIA VIRTUAL USB UART
void UART_Printf(const char* fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
		CDC_Transmit_FS( (uint8_t*)buff, strlen(buff)); 
    va_end(args);
}



