/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "utils.h"

#define CACHELINE         64
#define REGION_SIZE_LINES 8

__attribute__((aligned(CACHELINE))) uint64_t mem1[CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES];
__attribute__((aligned(CACHELINE))) uint64_t mem2[CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES];

int16_t saturate(int32_t x) {
    int16_t max = (1 << 15) - 1;
    int16_t min = -(1 << 15);
    if (x > max) {
        return max;
    } else if (x < min) {
        return min;
    } else {
        return x;
    }
}
char str[64] = "Hello world from hart\n";
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(int argc, char **argv) {
  
    CLEAR_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
    CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

    SET_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
    SET_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

    CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_NSTOP_MSK);
    CLEAR_BITS(UART0->TXCTRL, UART_STOPBITS_2);

    putstr("Initializing memory");
    for (size_t i = 0; i < sizeof(mem1)/sizeof(uint64_t); i++) {
        mem1[i] = i;
    }

    void* src_addr = mem1;
    void* dest_addr = mem2;
    uint64_t stride = CACHELINE;
    uint32_t count = REGION_SIZE_LINES;

    HAL_DMA_init_memcpy(DMA0, src_addr, dest_addr, stride, count);

    // wait for peripheral to complete
    while (!HAL_DMA_operation_complete(DMA0) && !HAL_DMA_operation_errored(DMA0)) {
      putstr("Waiting");
    }
    putstr("DONE");
    printnum(DMA0->STATUS);

    for (size_t i = 0; i < sizeof(mem1)/sizeof(uint64_t); i++) {
        uint64_t val = mem2[i], expected = mem1[i];
        if (val != expected) {
          putstr("Expected: ");
          printnum(expected);
          putstr("But got: ");
          printnum(val);
        }
    }

    putstr("Test complete");


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // printf("Attempting to write to I2C0: %d\n", counter);
    /* USER CODE END WHILE */
  }
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/*
 * Main function for secondary harts
 * 
 * Multi-threaded programs should provide their own implementation.
 */
void __attribute__((weak, noreturn)) __main(void) {
  while (1) {
   asm volatile ("wfi");
  }
}