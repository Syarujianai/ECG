#ifndef __FFT_H
#define __FFT_H
#include "stm32f10x.h"
/*************************�ṹ*******************************/

typedef struct

{
	float real;

	float img;

}complex;

/*************************����********************************/

void FFT(u16 *BUF);

/*************************����********************************/

//extern double Mag[80];//���ģֵ
//extern complex Ecg_FFT[256];//��������


#endif /* __FFT_H */
