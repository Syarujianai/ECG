#ifndef __FFT_H
#define __FFT_H
#include "stm32f10x.h"
/*************************结构*******************************/

typedef struct

{
	float real;

	float img;

}complex;

/*************************函数********************************/

void FFT(u16 *BUF);

/*************************变量********************************/

//extern double Mag[80];//输出模值
//extern complex Ecg_FFT[256];//输入序列


#endif /* __FFT_H */
