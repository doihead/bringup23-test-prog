#ifndef MIN_UTILS
#define MIN_UTILS
void LL_UART_transmit(UART_TypeDef *UARTx, const uint8_t *data, uint16_t size, uint32_t timeout);

void putstr(char* str);

void printnum(int64_t num);

#endif
