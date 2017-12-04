/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <stdio.h>
#include <string.h> 
#include "bsp_esp8266.h"
#include "test.h"
#include "bsp_adc.h"
#include "lcd.h" 
#include "bsp_date.h"
#include "bsp_rtc.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

#define BUF_Entrance 5
#define BUF1_store_Finish 0
#define BUF1_filter_Finish 1
#define BUF2_store_Finish 2
#define BUF2_filter_Finish 3

u16 DataFlag1=0,DataFlag2=0,DataFlag4=0;

/*时间结构体*/
struct rtc_time systmtime;

void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC1))	  //判断是否为DMA CH1 全部传输完成
	{
		 //DataCounter = DMA_GetCurrDataCounter(DMA1_Channel1);        //获取剩余长度,一般都为0,调试  
			DMA_ClearITPendingBit(DMA1_IT_GL1);                           //清除全部中断标志
			if((DMA_flag==BUF2_filter_Finish)||(DMA_flag==BUF_Entrance))
			{   
					DMA1_Channel1->CMAR = (u32)BUF2;          //重新设置内存地址		
					DMA_flag=BUF1_store_Finish;
				  GPIO_ResetBits ( GPIOD, GPIO_Pin_2 );
					DataFlag1+=2;
					LCD_ShowNum(20, 0, DataFlag1, 3, 16);		
			}
			if(DMA_flag==BUF1_filter_Finish)
			{
					DMA1_Channel1->CMAR = (u32)BUF1;          //重新设置内存地址		
					DMA_flag=BUF2_store_Finish;	
					GPIO_SetBits ( GPIOD, GPIO_Pin_2 );
					DataFlag2+=5;	
					LCD_ShowNum(20, 20, DataFlag2, 3, 16);					
			}
			DMA1_Channel1->CCR |= (u32)0x00000001;          //使能DMA	 			
     /*if(DMA_flag==BUF_Entrance)
        {   	
						DMA_Cmd(macADC_DMA_CHANNEL,DISABLE);      //先关闭DMA才能配置
						DMA1_Channel1->CMAR = (u32)BUF2;          //重新设置内存地址
						DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值
            DMA_Cmd(macADC_DMA_CHANNEL,ENABLE);       //重新启动DMA	
	          ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求					
						
					  DMA_flag=BUF1_store_Finish;					
            GPIO_ResetBits ( GPIOD, GPIO_Pin_2 );
					  DataFlag1+=2;
					  LCD_ShowNum(20, 0, DataFlag1, 3, 16);					
		    }		 
     else if(DMA_flag==BUF1_store_Finish)
        {               					
					
					  DMA_Cmd(macADC_DMA_CHANNEL,DISABLE);			//先关闭才能配置	
						DMA1_Channel1->CMAR = (u32)BUF1;          //重新设置内存地址
					  DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值					
					
			      DMA_flag=BUF2_store_Finish;					  
            GPIO_SetBits ( GPIOD, GPIO_Pin_2 );
            DataFlag2+=5;	
            LCD_ShowNum(20, 20, DataFlag2, 3, 16);					              					
        }
     else if(DMA_flag==BUF2_store_Finish)
        {   
					  //ADC_ExternalTrigConvCmd(macADCx,DISABLE);
					
					  DMA_Cmd(macADC_DMA_CHANNEL , DISABLE);					
						DMA1_Channel1->CMAR = (u32)BUF2;          //重新设置内存地址
					  DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值					

			      DMA_flag=BUF1_store_Finish;					
            GPIO_SetBits ( GPIOD, GPIO_Pin_2 );
            DataFlag3+=7;	
            LCD_ShowNum(20, 40, DataFlag3, 3, 16);					
        }*/			

	}
	DataFlag4+=10;
	LCD_ShowNum(20, 60, DataFlag4, 3, 16);

}


/*void TIM3_IRQHandler(void)
{
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);    //清除配置产生的中断标志
}*/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*void SysTick_Handler(void)
{
	TimingDelay_Decrement();	
}*/


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/**
  * @brief  This function handles macESP8266_USARTx Handler.
  * @param  None
  * @retval None
  */
void macESP8266_USART_INT_FUN ( void )
{	
	uint8_t ucCh;
	
	if ( USART_GetITStatus ( macESP8266_USARTx, USART_IT_RXNE ) != RESET )
	{
		ucCh  = USART_ReceiveData( macESP8266_USARTx );                                                  //收到数据，读取DR寄存器的值
		
		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //预留1个字节写结束符
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;

	}
	 	 
	if ( USART_GetITStatus( macESP8266_USARTx, USART_IT_IDLE ) == SET )                                 //数据帧接收完毕
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		
		ucCh = USART_ReceiveData( macESP8266_USARTx );                                                    //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
	
		ucTcpClosedFlag = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
		
  }	

}

void RTC_IRQHandler(void)
{
	  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	  {
	    /* Clear the RTC Second interrupt */
	    RTC_ClearITPendingBit(RTC_IT_SEC);
	
      to_tm(RTC_GetCounter(), &systmtime);/*把定时器的值转换为北京时间*/
			
			LCD_ShowString(0,104,80,16,16,"    -  -  ");	   
			LCD_ShowString(172,104,64,16,16,"  :  :  ");	 		 
      LCD_ShowNum(0,104,systmtime.tm_year,4,16);								  
			LCD_ShowNum(40,104,systmtime.tm_mon,2,16);									  
			LCD_ShowNum(64,104,systmtime.tm_mday,2,16);	 
			switch(systmtime.tm_wday)
			{
				case 0:
					LCD_ShowString(100,104,72,16,16,"Sunday   ");
					break;
				case 1:
					LCD_ShowString(100,104,72,16,16,"Monday   ");
					break;
				case 2:
					LCD_ShowString(100,104,72,16,16,"Tuesday  ");
					break;
				case 3:
					LCD_ShowString(100,104,72,16,16,"Wednesday");
					break;
				case 4:
					LCD_ShowString(100,104,72,16,16,"Thursday ");
					break;
				case 5:
					LCD_ShowString(100,104,72,16,16,"Friday   ");
					break;
				case 6:
					LCD_ShowString(100,104,72,16,16,"Saturday ");
					break;  
			}
			LCD_ShowNum(172,104,systmtime.tm_hour,2,16);									  
			LCD_ShowNum(196,104,systmtime.tm_min,2,16);									  
			LCD_ShowNum(220,104,systmtime.tm_sec,2,16);			
	
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	  }
}


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
