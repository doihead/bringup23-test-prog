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

char str[64] = "Hello world from hart\n";
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(int argc, char **argv) {
  /* USER CODE BEGIN 1 */
  uint16_t counter = 0;
    
  CLEAR_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  SET_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  SET_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_NSTOP_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_STOPBITS_2);

  // baudrate setting
  // f_baud = f_sys / (div + 1)
  // UART0->DIV = (SYS_CLK_FREQ / 115200) - 1;
  HAL_PWM_init(PWM0);
  HAL_PWM_setCompareValue(PWM0, 0, 8192);
  HAL_PWM_setCompareValue(PWM0, 1, 4096);
  HAL_PWM_setCompareValue(PWM0, 2, 4096);
  SET_BITS(PWM0->PWM_CFG, PWM_PWMCMP2CENTER_MSK);

  HAL_PWM_setCompareValue(PWM0, 3, 4096);
  CLEAR_BITS(PWM0->PWM_CFG, PWM_PWMCMP3GANG_MSK);

  HAL_PWM_enable(PWM0);

  GPIOA->INPUT_EN = 0x0;
  GPIOA->OUTPUT_EN = 0xFFFE;
  GPIOA->PUE = 0x0;
  GPIOA->DS = 0xFFFE;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    GPIOA->OUTPUT_VAL = counter;
    counter ++;
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