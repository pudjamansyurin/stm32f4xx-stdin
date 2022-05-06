# STM32F4xx standard input using UART (Circular DMA Mode)

## How to use the module
You should enable <b>USE_HAL_UART_REGISTER_CALLBACKS</b> macro

#### **`main.c`**
```c
#include "stm32f4xx-stdin/stdin.h"

#define USE_HAL_UART_REGISTER_CALLBACKS (1)
#define BUF_SZ (32)

/* Private variables */
static stdin_t hstdin;
static uint8_t Buffer[BUF_SZ];

/* Entry Point */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  
  /* Initialize serial layer */  
  stdin_init(&hstdin, &huart2, Buffer, BUF_SZ);
  stdin_set_callback(&hstdin, tinysh_chars_in);
  stdin_start(&hstdin);

  /* Super loop */
  while(1) {
    
  }
}
```


#### **`stm32f4xx_it.c`**
```c
/* USER CODE BEGIN Includes */
#include "stm32f4xx-stdin/stdin.h"
/* USER CODE END Includes */

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */

  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */
  stdin_irq_dma(&hstdin);
  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
  stdin_irq_uart(&hstdin);
  /* USER CODE END USART2_IRQn 1 */
}

```