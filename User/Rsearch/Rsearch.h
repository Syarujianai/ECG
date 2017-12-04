#ifndef __Rsearch_H
#define __Rsearch_H
#include "stm32f10x.h"
/*************************�ṹ*******************************/

typedef struct flag
{
	u16 rx;
	u16 ry;
}RSTORE;

/*************************����********************************/

void Rprocess(u16 *ecg);

/*************************����********************************/
extern u8 rnum;
extern RSTORE R[20];
extern float average_rr;

#endif /* __Rsearch_H */
