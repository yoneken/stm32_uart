/*
 * Cuart.cpp
 *
 *  Created on: 2017/11/27
 *      Author: yoneken
 */

#include "Cuart.h"
#include <string.h>

char rxbuf[64];
char rxindex = 0;
char rxbuf2[1];

extern UART_HandleTypeDef huart2;

void UartUtil_Init(void)
{
  HAL_UART_Receive_IT(&huart2, (uint8_t *)rxbuf2, sizeof(rxbuf2));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  char res = rxbuf2[0];
  HAL_UART_Receive_IT(&huart2, (uint8_t *)rxbuf2, sizeof(rxbuf2));

  if(res == '\r'){
    rxbuf[rxindex++] = res;
    rxbuf[rxindex++] = '\n';

    //HAL_UART_Transmit(&huart2, (uint8_t *)rxbuf, rxindex, 0xFF);
    HAL_UART_Transmit_IT(&huart2, (uint8_t *)rxbuf, rxindex);

    rxindex = 0;
  }else{
    rxbuf[rxindex] = res;

    if(rxindex <= sizeof(rxbuf)-2){
  	  rxindex++;
    }else{
  	  rxindex = 0;
    }
  }
}

/************************************/
/*  print with format               */
/************************************/
int printf(const char *format, ...)
{
  int  i = 0;
  va_list ap;

  va_start(ap, format);
  for (;;){
    char c, cc, n, l, pc, flag;
    signed char m;
    m=0;
    n=l=pc=flag=0;
    while((c = *format++) != '%') {
      if(!c) {
        va_end(ap);
        return i;
      }else{
        HAL_UART_Transmit(&huart2, (uint8_t *)&c, 1, 0xFF);
        i++;
      }
    }
    if(*format==' '||*format=='+'){
      pc=*format++;
    }
    if(*format=='0'){
      flag|=0x04;
      format++;
    }
    cc = *format;
    if(('0'<cc)&&(cc<='9')){
      flag|=0x08;
      n = cc - '0';
      format++;
    }
    if(*format=='.'){
      format++;
      cc = *format;
      if(('0'<cc)&&(cc<='9')){
        m = cc - '0';
        format++;
      }
    }

    switch(c = *format++){
      char scratch[32], *ptr;
      long val, base;
      double vald;
      case 'c':
        c = va_arg(ap, char);
      default:
        i++;
        HAL_UART_Transmit(&huart2, (uint8_t *)&c, 1, 0xFF);
        continue;
      case 's':
        ptr = va_arg(ap, char *);
        HAL_UART_Transmit(&huart2, (uint8_t *)ptr, strlen(ptr), 0xFF);
        i += strlen(ptr);
        continue;
      case 'o':
        val = va_arg(ap, long);
        base = 8;
        goto CONV_LOOP;
      case 'd':
        val = va_arg(ap, long);
        base = 10;
        if(val < 0){
          pc = '-';
          val = - val;
        }
        goto CONV_LOOP;
      case 'b':
        val = va_arg(ap, unsigned long);
        base = 2U;
        goto CONV_LOOP;
      case 'X':
        flag|=0x02;
      case 'x':
        val = va_arg(ap, unsigned long);
        base = 16U;
        goto CONV_LOOP;
      case 'f':
        flag|=0x10;
        if(m==0) m=5;
        vald = va_arg(ap, double);
        base = 10L;
        if(vald < 0.0){
          pc = '-';
          vald = - vald;
        }
        for(i=0;i<m;i++) vald*=10.0;
        val = (long)vald;
        goto CONV_LOOP;
      CONV_LOOP:
      l = n+m+(flag&0x10?1:0);
      for(i=0;i<31;i++) scratch[i] = flag&0x04 ? '0' : ' ';
      ptr = &(scratch[31]);
      *ptr = 0;
      do{
        if(flag&0x10) if(m--==0) *--ptr = '.';
        cc = (char)(val % base) + '0';
        if (cc > '9') cc += (flag&0x02 ? 'A' : 'a')  - '9' - 1;
        *--ptr = cc;
        val /= base;
      }while(val);
      if((flag&0x10)&&(m>=0)){
        do{
          if(m--==0) *--ptr='.';
          *--ptr='0';
        }while(m>=0);
      }
      if(flag&0x08) ptr = scratch + 31 - l;
      if(pc) *--ptr = pc;
      HAL_UART_Transmit(&huart2, (uint8_t *)ptr, strlen(ptr), 0xFF);
      i += strlen(ptr);
    }
  }
}

class Cuart {
public:
	Cuart();
	virtual ~Cuart();
};

Cuart::Cuart() {
	// TODO Auto-generated constructor stub

}

Cuart::~Cuart() {
	// TODO Auto-generated destructor stub
}

