#include "utils.h"

void LL_UART_transmit(UART_TypeDef *UARTx, const uint8_t *data, uint16_t size, uint32_t timeout) {
  while (size > 0) {
    while (READ_BITS(UARTx->TXDATA, UART_TXDATA_FULL_MSK)) {
      // return TIMEOUT;
    }
    UARTx->TXDATA = *data;
    data += sizeof(uint8_t);
    size -= 1;
  }
}

static inline void printchar(char c) {
  while (READ_BITS(UART0->TXDATA, UART_TXDATA_FULL_MSK));
  UART0->TXDATA = c;
}

void putstr(char* str) {
  while (str[0] != 0) {
    while (READ_BITS(UART0->TXDATA, UART_TXDATA_FULL_MSK));
    UART0->TXDATA = str[0];
    str += 1;
  }
}

void printnum(int64_t num) {
  if (num == 0) {
    printchar('0');
    return;
  }
  char negative = num < 0;
  if (negative)
    num = -num;

  char charbuf[32];
  int counter = 0;
  while(num != 0) {
    charbuf[counter] = num % 10 + '0';
    num /= 10;
    counter += 1;
  }
  if (negative)
      printchar('-');
  for(counter -= 1; counter >= 0; counter --) {
    printchar(charbuf[counter]);
  }
}
