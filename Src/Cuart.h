/* * Cuart.h
 *
 *  Created on: 2017/11/27
 *      Author: yoneken
 */
#ifndef _CUART_H_
#define _CUART_H_

#include <stdarg.h>
#include "stm32f3xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void UartUtil_Init(UART_HandleTypeDef *hnd);
void UartUtil_putc(UART_HandleTypeDef *hnd, char c);
void UartUtil_puts(UART_HandleTypeDef *hnd, char str[]);
int m_printf(UART_HandleTypeDef *hnd, const char *format, ...);

void *new_uart(UART_HandleTypeDef *hnd);
void delete_uart(void *cuart);
int uart_printf(void *cuart, const char *format, ...);

#ifdef __cplusplus
};
#endif


#endif /* _CUART_H_ */
