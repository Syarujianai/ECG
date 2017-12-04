#ifndef __ADC_H
#define	__ADC_H


#include "stm32f10x.h"

// ע�⣺����ADC�ɼ���IO����û�и��ã�����ɼ���ѹ����Ӱ��
/********************ADC1����ͨ�������ţ�����**************************/
#define    macADC_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    macADC_CLK                       RCC_APB2Periph_ADC1

#define    macADC_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd
#define    macADC_GPIO_CLK                  RCC_APB2Periph_GPIOC  
#define    macADC_PORT                      GPIOC

// PC1-ͨ��11 ����IO
#define    macADC_PIN                       GPIO_Pin_4
#define    macADC_CHANNEL                   ADC_Channel_14

#define    macADCx                          ADC1
#define    macADC_DMA_CHANNEL               DMA1_Channel1


/**************************��������********************************/
void               ADCx_Init                               (void);
/**************************��������********************************/
extern __IO u16 BUF1[256];  //����1��
extern __IO u16 BUF2[256];  //����2��
extern __IO u8 DMA_flag;     //DMA��־λ

#endif /* __ADC_H */

