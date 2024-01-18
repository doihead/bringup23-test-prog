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
#include "hal_dma.h"
#include "main.h"
#include <stdio.h>

#define CACHELINE         64
#define REGION_SIZE_LINES 16384
#define STRIDE            64

// __attribute__((aligned(CACHELINE))) uint64_t mem1[CACHELINE /
// sizeof(uint64_t) * REGION_SIZE_LINES];
// __attribute__((aligned(CACHELINE))) uint64_t mem2[CACHELINE /
// sizeof(uint64_t) * REGION_SIZE_LINES];

uint64_t *mem1 = 0x80000000;
uint64_t *mem2 = 0x80100000;

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
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(int argc, char **argv) {

  UART_InitTypeDef UART_init_config;
  UART_init_config.baudrate = 115200;
  UART_init_config.mode = UART_MODE_TX_RX;
  UART_init_config.stopbits = UART_STOPBITS_2;
  HAL_UART_init(UART0, &UART_init_config);

  printf("Begining Test Size: %lu, Stride: %lu\r\n", REGION_SIZE_LINES, STRIDE);
  puts("Initializing memory");
  for (size_t i = 0; i < CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES;
       i++) {
    mem1[i] = i;
  }
  for (size_t i = 0; i < CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES;
       i++) {
    mem2[i] = 0;
  }
  void *src_addr = mem1;
  void *dest_addr = mem2;
  uint64_t stride = CACHELINE;
  uint32_t count = REGION_SIZE_LINES;
  uint64_t start_time;
  uint64_t end_time;

  // start_time = CLINT->MTIME;
  // for (size_t i = 0; i < CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES;
  // i++) {
  //   mem2[i] = mem1[i];
  // }
  // end_time = CLINT->MTIME;
  // printf("Naive took: %lu cycles\r\n", end_time-start_time);

  start_time = CLINT->MTIME;

  HAL_DMA_init_memcpy(DMA0, src_addr, dest_addr, stride, count);

  // HAL_DMA_init_memcpy(DMA0, src_addr, dest_addr, stride*4, count/4);
  // HAL_DMA_init_memcpy(DMA1, src_addr + CACHELINE, dest_addr + CACHELINE,
  // stride*4, count/4); HAL_DMA_init_memcpy(DMA2, src_addr + 2*CACHELINE,
  // dest_addr + 2*CACHELINE, stride*4, count/4); HAL_DMA_init_memcpy(DMA3,
  // src_addr + 3*CACHELINE, dest_addr + 3*CACHELINE, stride*4, count/4);

  // wait for peripheral to complete
  while (HAL_DMA_operation_inprogress(DMA0) ||
         HAL_DMA_operation_inprogress(DMA1) ||
         HAL_DMA_operation_inprogress(DMA2) ||
         HAL_DMA_operation_inprogress(DMA3))
    ;

  end_time = CLINT->MTIME;
  printf("DONE with code: %lu after %lu cycles\r\n", DMA0->STATUS,
         end_time - start_time);

  for (size_t i = 0; i < CACHELINE / sizeof(uint64_t) * REGION_SIZE_LINES;
       i++) {
    uint64_t val = mem2[i], expected = mem1[i];
    if (val != expected) {
      printf("Expected: %lu but got: %lu\r\n", expected, val);
    }
  }
  puts("Test complete");

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
    asm volatile("wfi");
  }
}