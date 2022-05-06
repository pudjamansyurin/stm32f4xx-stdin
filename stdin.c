/*
 * stdin.c
 *
 *  Created on: Apr 19, 2022
 *      Author: pudja
 */
#include "stdin.h"
#include <string.h>

#define HDMA(_X_) ((_X_)->huart->hdmarx)

/* Private function prototypes
 * --------------------------------------------*/
static void check_buffer(stdin_t *in);
static void fill_buffer(stdin_t *in, uint16_t pos, uint16_t size);

/* Public function implementations
 * --------------------------------------------*/
void stdin_init(stdin_t *in, UART_HandleTypeDef *uart, uint8_t *buffer, uint16_t size)
{
  in->huart = uart;
  in->buffer = buffer;
  in->size = size;

  stdin_flush(in);
}

void stdin_set_callback(stdin_t *in, stdin_callback_t cb)
{
  in->callback = cb;
}

void stdin_flush(stdin_t *in)
{
  memset(in->buffer, 0, in->size);
  in->pos = 0;
}

HAL_StatusTypeDef stdin_start(stdin_t *in)
{
  HAL_StatusTypeDef status;

  /* Enable interrupts */
  __HAL_UART_ENABLE_IT(in->huart, UART_IT_IDLE);
  __HAL_DMA_ENABLE_IT(HDMA(in), DMA_IT_TC);
  __HAL_DMA_ENABLE_IT(HDMA(in), DMA_IT_HT);

  /* Start receiving UART in DMA mode */
  status = HAL_UART_Receive_DMA(in->huart, in->buffer, in->size);
  return (status);
}

HAL_StatusTypeDef stdin_stop(stdin_t *in)
{
  HAL_StatusTypeDef status;

  status = HAL_UART_DMAStop(in->huart);
  return (status);
}

void stdin_irq_dma(stdin_t *in)
{
  /* Handle HT interrupt */
  if (__HAL_DMA_GET_IT_SOURCE(HDMA(in), DMA_IT_HT))
  {
    __HAL_DMA_CLEAR_FLAG(HDMA(in), __HAL_DMA_GET_HT_FLAG_INDEX(HDMA(in)));
    check_buffer(in);
  }

  /* Handle TC interrupt */
  else if (__HAL_DMA_GET_IT_SOURCE(HDMA(in), DMA_IT_TC))
  {
    __HAL_DMA_CLEAR_FLAG(HDMA(in), __HAL_DMA_GET_TC_FLAG_INDEX(HDMA(in)));
    check_buffer(in);
  }

  /* Handle ERROR interrupt */
  else
  {
    __HAL_DMA_CLEAR_FLAG(HDMA(in), __HAL_DMA_GET_TE_FLAG_INDEX(HDMA(in)));
    __HAL_DMA_CLEAR_FLAG(HDMA(in), __HAL_DMA_GET_FE_FLAG_INDEX(HDMA(in)));
    __HAL_DMA_CLEAR_FLAG(HDMA(in), __HAL_DMA_GET_DME_FLAG_INDEX(HDMA(in)));

    HAL_UART_Receive_DMA(in->huart, in->buffer, in->size);
  }
}

void stdin_irq_uart(stdin_t *in)
{
  if (__HAL_UART_GET_FLAG(in->huart, UART_FLAG_IDLE))
  {
    __HAL_UART_CLEAR_IDLEFLAG(in->huart);
    check_buffer(in);
  }
}

/* Private function implementations
 * --------------------------------------------*/
static void check_buffer(stdin_t *in)
{
  uint16_t pos, new;

  /* Calculate current position in buffer */
  new = __HAL_DMA_GET_COUNTER(HDMA(in));
  pos = in->size - new;

  /* Check change in received data */
  if (pos != in->pos)
  {
    if (pos > in->pos)
      /* Current position is over previous one */
      /* We are in "linear" mode */
      /* Process data directly by subtracting "pointers" */
      fill_buffer(in, in->pos, pos - in->pos);
    else
    {
      /* We are in "overflow" mode */
      /* First process data to the end of buffer */
      fill_buffer(in, in->pos, in->size - in->pos);
      /* Check and continue with beginning of buffer */
      if (pos > 0)
        fill_buffer(in, 0, pos);
    }
  }

  /* Check and manually update if we reached end of buffer */
  in->pos = (pos == in->size) ? 0 : pos;
}

static void fill_buffer(stdin_t *in, uint16_t pos, uint16_t size)
{
  if (NULL == in->callback)
    return;

  in->callback(&in->buffer[pos], size);
}

