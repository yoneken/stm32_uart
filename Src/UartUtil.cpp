/*
 * UartUtil.cpp
 *
 *  Created on: 2017/11/27
 *      Author: yoneken
 */

#include <string.h>
#include "UartUtil.h"

class UartUtil;

typedef struct uart_chain{
  UartUtil *c;
  UART_HandleTypeDef *hnd;
  uart_chain *next;
};

uart_chain *chain = NULL;

class UartUtil {
public:
  UartUtil(UART_HandleTypeDef *hnd);
  virtual ~UartUtil();
  int printf(const char *format, ...);
  void flush(void);
  void recv(void);

private:
  char rxbuf[64];
  volatile char rxindex = 0;
  char rxbuf2[1];

  char txbuf[64];
  volatile char tind_write = 0;
  volatile char tind_flush = 0;

  UART_HandleTypeDef *huart;

  void putcc(char);
  void puts(char[]);
};

UartUtil::UartUtil(UART_HandleTypeDef *hnd){
  huart = hnd;
  HAL_UART_Receive_IT(huart, (uint8_t *)rxbuf2, sizeof(rxbuf2));
}

UartUtil::~UartUtil(){
	// TODO Auto-generated destructor stub
}

void UartUtil::flush(void)
{
  static uint8_t mutex = 0;
  int blen = sizeof(txbuf);

  if((huart->gState != HAL_UART_STATE_BUSY_TX) && (mutex == 0)){
    mutex = 1;
    if((tind_flush == blen)&&(tind_write != blen)) tind_flush = 0;

    if(tind_write > tind_flush){
      HAL_UART_Transmit_IT(huart, (uint8_t *)&(txbuf[tind_flush]), tind_write - tind_flush);
      tind_flush += tind_write - tind_flush;
    }else if(tind_write != tind_flush){
      if(tind_flush != blen){
        HAL_UART_Transmit_IT(huart, (uint8_t *)&(txbuf[tind_flush]), blen - tind_flush);
        tind_flush += blen - tind_flush;
      }else{
        HAL_UART_Transmit_IT(huart, (uint8_t *)txbuf, tind_write);
        tind_flush = tind_write;
      }
    }

    mutex = 0;
  }
}

void UartUtil::putcc(char c)
{
  int blen = sizeof(txbuf);

  if(tind_write < blen){
    txbuf[tind_write] = c;
    tind_write++;
  }else{
    txbuf[0] = c;
    tind_write = 1;
  }

  flush();
}

void UartUtil::puts(char str[])
{
  int n = strlen(str);
  int blen = sizeof(txbuf);
  n = n <= blen ? n : blen;

  int n_tail = n <= blen - tind_write ? n : blen - tind_write;
  strncpy(&(txbuf[tind_write]), str, n_tail);
  tind_write = (tind_write + n)% blen;

  if(n != n_tail){
	strncpy(txbuf, &(str[n_tail]), n - n_tail);
  }

  flush();
}

int UartUtil::printf(const char *format, ...){
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
        putcc(c);
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
        c = va_arg(ap, int);
      default:
        i++;
        putcc(c);
        continue;
      case 's':
        ptr = va_arg(ap, char *);
        puts(ptr);
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
      puts(ptr);
      i += strlen(ptr);
    }
  }
}

void UartUtil::recv(void)
{
  HAL_UART_Receive_IT(huart, (uint8_t *)rxbuf2, sizeof(rxbuf2));

  char res = rxbuf2[0];

  if(res == '\r'){
    rxbuf[rxindex++] = res;
    rxbuf[rxindex++] = '\n';

    char str[64];
    strncpy(str, rxbuf, rxindex);
    printf("Recv: %s\r\n", str);

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

void *new_uart(UART_HandleTypeDef *hnd)
{
  UartUtil *c = new UartUtil(hnd);

  uart_chain *n = new uart_chain();
  n->c = c;
  n->hnd = hnd;
  n->next = NULL;

  if(chain == NULL){
    chain = n;
  }else{
    // search last
    uart_chain *i = chain;
    while(i->next != NULL) i = i->next;
    i = n;
  }

  return c;
}

void delete_uart(void *cuart)
{
  if(chain == NULL){
    return;
  }else{
    // search node
    uart_chain *i = chain;
    uart_chain *prev;
    while((i->c != cuart) && (i->next != NULL)){
      prev = i;
      i = i->next;
    }
    if(i == NULL) return;

    prev->next = i->next;

    delete (UartUtil *)(i->c);
    delete i;
  }
}

int uart_printf(void *cuart, const char *format, ...)
{
  ((UartUtil *)cuart)->printf(format);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *hnd)
{
  // search node
  uart_chain *i = chain;

  while((i != NULL) && (i->hnd != hnd)){
    i = i->next;
  }
  if(i == NULL) return;
  i->c->flush();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *hnd)
{
  // search node
  uart_chain *i = chain;

  while((i != NULL) && (i->hnd != hnd)){
    i = i->next;
  }
  if(i == NULL) return;

  i->c->recv();
}
