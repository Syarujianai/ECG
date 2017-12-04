/**
  ******************************************************************************
  * @file    RingBuf.c
  * @author  wanxiao
  * @version V1.0
  * @date    2016-08-17
  * @brief   mplement a circular buffer,you can read and write data in the 
	buffer zone.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */  
#include "RingBuf.h"
//#include "fft.h"

__IO u16 LCD_DrawBuf[512];             //用于LCD流动显示与FFT变换 
volatile int writeldx_lcd = 0;
//volatile int writeldx_fft = 0;

int next_data_handle1(int addr1)
{
	return ((addr1 + 1) == MAXSIZE) ? 0 : (addr1 + 1);
}

/*int next_data_handle2(int addr2)
{
	return ((addr2 + 1) == 64) ? 0 : (addr2 + 1);
}*/


void write_int_data_LCD(u16 data1)
{
	*(LCD_DrawBuf+writeldx_lcd)=data1;
	writeldx_lcd = next_data_handle1(writeldx_lcd);
}

/*void write_float_data_FFT(float data2)
{
  (AD_FFT+writeldx_fft)->real=data2;
	writeldx_fft = next_data_handle2(writeldx_fft);
}*/
