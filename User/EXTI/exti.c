#include "exti.h"
#include "key.h"
#include "delay.h"
#include "ff.h"
#include "exfuns.h"
#include <stdio.h>
#include "bsp_date.h"
#include "bsp_adc.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//外部中断 驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/01  
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved	  
////////////////////////////////////////////////////////////////////////////////// 	  
u8 Sd_Store_flag=0;
u8 Sd_Review_flag=0;
u8 Esp_Send_flag=1;
//static char Filname[20];
extern struct rtc_time systmtime;
extern u8 ID,ID1,ID2,ID3;//判断第几次画图
 
//外部中断初始化函数
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟

	  KEY_Init();//初始化按键对应io模式

    //GPIOC.5 中断线以及中断初始化配置
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

    //GPIOA.15	  中断线以及中断初始化配置
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
 
 
   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
 
}

/*需要修改:1.对res进行判断2.仅关闭文件指针?*/
 void EXTI9_5_IRQHandler(void)
{			
	delay_ms(10);   //消抖			 
	if(KEY0==0)	{
		u8 res;
		if(Sd_Store_flag==0)
		{
			Sd_Store_flag=1;
			Esp_Send_flag=0;
			/*储存中断里执行*/
			//to_tm(RTC_GetCounter(), &systmtime);/*把定时器的值转换为北京时间*/
			//sprintf(Filname,"0:\\16-08-23.txt");
			//sprintf(Filname,"0:\\%02d:%02d:%02d.txt",systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);
			//res=f_open(file,Filname,FA_WRITE|FA_CREATE_ALWAYS);
		}	
		else if(Sd_Store_flag==1)
		{
			Sd_Store_flag=0;
			Esp_Send_flag=1;
			ID2=0;
			/*关闭当前文件指针，已执行f_SYNC操作*/
		  res=f_close(file);
		}			
	}
 	 EXTI_ClearITPendingBit(EXTI_Line5);    //清除LINE5上的中断标志位  
}

void EXTI15_10_IRQHandler(void)
{
  delay_ms(10);    //消抖			 
  if(KEY1==0)	{
		u8 res;
		if(Sd_Review_flag==0)
		{
			DMA_Cmd(macADC_DMA_CHANNEL,DISABLE);			//先关闭才能配置
			Sd_Review_flag=1;
			Esp_Send_flag=0;
			//res=f_open(file,Filname,FA_READ|FA_OPEN_EXISTING);
		}	
		else if(Sd_Review_flag==1)
		{
			DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值
			DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //重新使能DMA
      ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求！！！
			Sd_Review_flag=0;
			Esp_Send_flag=1;
			res=f_close(file);
			ID1=0;
			ID=0;//ID复位
		}		
	}
	 EXTI_ClearITPendingBit(EXTI_Line15);  //清除LINE15线路挂起位
}
