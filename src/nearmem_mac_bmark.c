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

#define CACHELINE         64
#define REGION_SIZE_LINES 32
#define STRIDE (CACHELINE)

__attribute__((aligned(CACHELINE*4))) int8_t mem1[REGION_SIZE_LINES][CACHELINE / sizeof(int8_t)];

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
  /* USER CODE BEGIN 1 */
  // uint8_t counter = 0;
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  // set up UART registers
  UART_InitTypeDef UART_init_config;
  UART_init_config.baudrate = 115200;
  UART_init_config.mode = UART_MODE_TX_RX;
  UART_init_config.stopbits = UART_STOPBITS_2;
  HAL_UART_init(UART0, &UART_init_config);
  

  // baudrate setting
  // f_baud = f_sys / (div + 1)
  // UART0->DIV = (SYS_CLK_FREQ / 115200) - 1;
  puts("Initializing memory");
  for (size_t i = 0; i < REGION_SIZE_LINES; i++) {
      for (size_t j = 0; j < CACHELINE/sizeof(int8_t); j++) {
          mem1[i][j] = i + j;
      }
  }

  int8_t operandReg[64];
  for (size_t i = 0; i < sizeof(operandReg)/sizeof(uint8_t); i++) {
      operandReg[i] = (10)*((i > sizeof(operandReg)/sizeof(uint8_t) + 1) ? 1 : -1);
  }

  _Static_assert(sizeof(operandReg) == 64, "opreg size");

  _Static_assert(STRIDE % CACHELINE == 0, "stride not aligned");
  uint32_t count = REGION_SIZE_LINES / (STRIDE / CACHELINE);

  int16_t expected[32];
  uint64_t start_time;
  uint64_t end_time;

  start_time = CLINT->MTIME;

  for (size_t i = 0; i < count; i++) {
      int sum = 0;
      for (size_t j = 0; j < sizeof(operandReg)/sizeof(uint8_t); j++) {
          sum += (int16_t)operandReg[j] * (int16_t)mem1[i*(STRIDE/CACHELINE)][j];
      }
      expected[i] = saturate(sum);
  }
  end_time = CLINT->MTIME;

  printf("Naive took: %lu cycles\r\n", end_time-start_time);

  void* src_addr = mem1;
  int16_t result[32];

  uint64_t stride = STRIDE;

  puts("Performing MAC");
  start_time = CLINT->MTIME;

  // HAL_DMA_init_MAC(DMA0, src_addr, operandReg, stride, count);

  HAL_DMA_init_MAC(DMA0, src_addr, operandReg, stride*4, count/4);
  HAL_DMA_init_MAC(DMA1, src_addr +   CACHELINE, operandReg, stride*4, count/4);
  HAL_DMA_init_MAC(DMA2, src_addr + 2*CACHELINE, operandReg, stride*4, count/4);
  HAL_DMA_init_MAC(DMA3, src_addr + 3*CACHELINE, operandReg, stride*4, count/4);

  // wait for peripheral to complete
  HAL_DMA_get_MAC_result(DMA0, result, count/4);
  HAL_DMA_get_MAC_result(DMA1, result + 8, count/4);
  HAL_DMA_get_MAC_result(DMA2, result + 16, count/4);
  HAL_DMA_get_MAC_result(DMA3, result + 24, count/4);

  end_time = CLINT->MTIME;
  printf("DMA took: %lu cycles\r\n", end_time-start_time);

  for (size_t i = 0; i < count; i++) {
      if (expected[i] != result[i]) {
          printf("Expected: %lu but got %lu\r\n", expected[i], result[i]);
      }
  }
  // printf("Dumping...\n");

  // for (size_t i = 0; i < 8; i++) {
  //     printf("\t%016" PRIx64 "\n", DMA1->destReg[i]);
  // }
  

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
   asm volatile ("wfi");
  }
}