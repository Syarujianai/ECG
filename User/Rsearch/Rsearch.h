#ifndef __Rsearch_H
#define __Rsearch_H
#include "stm32f10x.h"
/*************************结构*******************************/

typedef struct flag
{
	u16 rx;
	u16 ry;
}RSTORE;

/*************************函数********************************/

void Rprocess(u16 *ecg);

/*************************变量********************************/
extern u8 rnum;
extern RSTORE R[20];
extern float average_rr;

#endif /* __Rsearch_H */
