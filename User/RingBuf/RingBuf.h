#ifndef  __RingBuf_H
#define	 __RingBuf_H

#include "stm32f10x.h"
#define MAXSIZE 512

void write_int_data_LCD(u16);
//void write_float_data_FFT(float);

extern __IO u16 LCD_DrawBuf[MAXSIZE];             //����LCD������ʾ��FFT�任 
#endif /*__RingBuf_H */

/****************************************End Of File*************************************************/
