#include "exti.h"
#include "key.h"
#include "delay.h"
#include "ff.h"
#include "exfuns.h"
#include <stdio.h>
#include "bsp_date.h"
#include "bsp_adc.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//�ⲿ�ж� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/01  
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved	  
////////////////////////////////////////////////////////////////////////////////// 	  
u8 Sd_Store_flag=0;
u8 Sd_Review_flag=0;
u8 Esp_Send_flag=1;
//static char Filname[20];
extern struct rtc_time systmtime;
extern u8 ID,ID1,ID2,ID3;//�жϵڼ��λ�ͼ
 
//�ⲿ�жϳ�ʼ������
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��

	  KEY_Init();//��ʼ��������Ӧioģʽ

    //GPIOC.5 �ж����Լ��жϳ�ʼ������
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

    //GPIOA.15	  �ж����Լ��жϳ�ʼ������
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
 
 
   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
 
}

/*��Ҫ�޸�:1.��res�����ж�2.���ر��ļ�ָ��?*/
 void EXTI9_5_IRQHandler(void)
{			
	delay_ms(10);   //����			 
	if(KEY0==0)	{
		u8 res;
		if(Sd_Store_flag==0)
		{
			Sd_Store_flag=1;
			Esp_Send_flag=0;
			/*�����ж���ִ��*/
			//to_tm(RTC_GetCounter(), &systmtime);/*�Ѷ�ʱ����ֵת��Ϊ����ʱ��*/
			//sprintf(Filname,"0:\\16-08-23.txt");
			//sprintf(Filname,"0:\\%02d:%02d:%02d.txt",systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);
			//res=f_open(file,Filname,FA_WRITE|FA_CREATE_ALWAYS);
		}	
		else if(Sd_Store_flag==1)
		{
			Sd_Store_flag=0;
			Esp_Send_flag=1;
			ID2=0;
			/*�رյ�ǰ�ļ�ָ�룬��ִ��f_SYNC����*/
		  res=f_close(file);
		}			
	}
 	 EXTI_ClearITPendingBit(EXTI_Line5);    //���LINE5�ϵ��жϱ�־λ  
}

void EXTI15_10_IRQHandler(void)
{
  delay_ms(10);    //����			 
  if(KEY1==0)	{
		u8 res;
		if(Sd_Review_flag==0)
		{
			DMA_Cmd(macADC_DMA_CHANNEL,DISABLE);			//�ȹرղ�������
			Sd_Review_flag=1;
			Esp_Send_flag=0;
			//res=f_open(file,Filname,FA_READ|FA_OPEN_EXISTING);
		}	
		else if(Sd_Review_flag==1)
		{
			DMA_SetCurrDataCounter(DMA1_Channel1,256);//�������õ�ǰ���ݼ�����ֵ
			DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //����ʹ��DMA
      ADC_DMACmd(macADCx, ENABLE);              //����ʹ��ADC DMA ���󣡣���
			Sd_Review_flag=0;
			Esp_Send_flag=1;
			res=f_close(file);
			ID1=0;
			ID=0;//ID��λ
		}		
	}
	 EXTI_ClearITPendingBit(EXTI_Line15);  //���LINE15��·����λ
}
