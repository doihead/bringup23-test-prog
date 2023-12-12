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
#define REGION_SIZE_LINES 16 
#define STRIDE (4 * CACHELINE)

__attribute__((aligned(CACHELINE))) int8_t mem1[REGION_SIZE_LINES][CACHELINE / sizeof(int8_t)];

__attribute__((aligned(CACHELINE))) int8_t mem2[REGION_SIZE_LINES][CACHELINE / sizeof(int8_t)];

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

  // set up GPIO registers
  // GPIO_InitTypeDef GPIO_init_config;
  // GPIO_init_config.mode = GPIO_MODE_OUTPUT;
  // GPIO_init_config.pull = GPIO_PULL_NONE;
  // GPIO_init_config.drive_strength = GPIO_DS_STRONG;
  // HAL_GPIO_init(GPIOA, &GPIO_init_config, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

  // set up UART registers
  // UART_InitTypeDef UART_init_config;
  // UART_init_config.baudrate = 115200;
  // UART_init_config.mode = UART_MODE_TX_RX;
  // UART_init_config.stopbits = UART_STOPBITS_2;
  // HAL_UART_init(UART0, &UART_init_config);
  
  CLEAR_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  SET_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  SET_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_NSTOP_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_STOPBITS_2);

  // baudrate setting
  // f_baud = f_sys / (div + 1)
  // UART0->DIV = (SYS_CLK_FREQ / 115200) - 1;
  LL_UART_transmit(UART0, str, 64, 0);
  printnum(0);
  printnum(-84);
  putstr("Initializing memory");
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
  for (size_t i = 0; i < count; i++) {
      int sum = 0;
      for (size_t j = 0; j < sizeof(operandReg)/sizeof(uint8_t); j++) {
          sum += (int16_t)operandReg[j] * (int16_t)mem1[i*(STRIDE/CACHELINE)][j];
      }
      expected[i] = saturate(sum);
  }
 
  void* src_addr = mem1;
  int16_t result[32];

  uint64_t stride = STRIDE;

  putstr("Performing DMA");

  HAL_DMA_init_MAC(DMA0, src_addr, operandReg, stride, count);
  // wait for peripheral to complete
  HAL_DMA_get_MAC_result(DMA0, result, 32);

  for (size_t i = 0; i < count; i++) {
      if (expected[i] != result[i]) {
          putstr("Expected: ");
          printnum(expected[i]);
          putstr("But got: ");
          printnum(result[i]);
      }
  }
  // printf("Dumping...\n");

  // for (size_t i = 0; i < 8; i++) {
  //     printf("\t%016" PRIx64 "\n", DMA1->destReg[i]);
  // }
  

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



