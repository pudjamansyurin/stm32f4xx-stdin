/*
 * stdin.h
 *
 *  Created on: Apr 19, 2022
 *      Author: pudja
 */

#ifndef SRC_STDIN_STDIN_H_
#define SRC_STDIN_STDIN_H_

#include "stm32f4xx_hal.h"

/* Exported types
 * --------------------------------------------*/
typedef void (*stdin_callback_t)(uint8_t *pbuffer, uint16_t size);

typedef struct
{
  UART_HandleTypeDef *huart;
  stdin_callback_t callback;
  uint8_t *buffer;
  uint16_t size;
  uint16_t pos;
} stdin_t;

/* Public function prototypes
 * --------------------------------------------*/
void stdin_init(stdin_t *in, UART_HandleTypeDef *uart, uint8_t *buffer, uint16_t size);
void stdin_set_callback(stdin_t *in, stdin_callback_t cb);
void stdin_flush(stdin_t *in);
HAL_StatusTypeDef stdin_start(stdin_t *in);
HAL_StatusTypeDef stdin_stop(stdin_t *in);
void stdin_irq_dma(stdin_t *in);
void stdin_irq_uart(stdin_t *in);

#endif /* SRC_STDIN_STDIN_H_ */
