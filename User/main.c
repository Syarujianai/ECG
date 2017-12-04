/**
  ******************************************************************************
  * @file    main.c
  * @author  Syaru
  * @version V1.0
  * @date    2016-08-17
  * @brief   心电波形的采集、储存、在LCD上的显示及WIFI传输
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#include <string.h>
#include "stm32f10x.h"
#include "bsp_esp8266.h"
#include "test.h"
#include "bsp_adc.h"
#include "lcd.h"
#include "change.h"
#include "fft.h"
#include "RingBuf.h"
#include "Rsearch.h"
#include "bsp_rtc.h"
#include "delay.h"
#include "exfuns.h"
#include "diskio.h"
#include "ff.h"
#include "malloc.h"
#include "MMC_SD.h"
#include "exti.h"
#include <stdio.h>
#include "bsp_date.h"

//DMA 缓冲及主函数处理标志
#define BUF_Entrance 5
#define BUF1_store_Finish 0
#define BUF1_filter_Finish 1
#define BUF2_store_Finish 2
#define BUF2_filter_Finish 3

//__IO char  Send_char_Data[4];              //发送数组
u16 Draw_Data_int;                           //用于画图
__IO u16 i,j;
u8 mainflag1=0,mainflag2=0;                
u8 res;                                    //命令返回值  
extern double Mag[80];                     //FFT输出序列
extern double Mag_total;                   //计算百分比用
float rr_show, heart_rate;
double rate=0;
u8 ID,ID1,ID2,ID3=0;//exti.C执行相关操作
static char Filname[20];
/*时间结构体*/
extern struct rtc_time systmtime;

//模式选择标志
extern u8 Sd_Store_flag;
extern u8 Sd_Review_flag;
extern u8 Esp_Send_flag;

//滤波器变量
float w0[3]={0};
float w1[3]={0};
float x0=0;
float y0=0,y1=0;

__IO float IIR_Filter_Data[256];           //缓存数组，储存滤波之后的时域信号 float类型

//50HZ陷波器结构系数
const float IIR_50Notch_B[3] = {
   0.9023977442,
	-0.5577124773,
	 0.9023977442
};

const float IIR_50Notch_A[3] = {
	 1,
	-0.5577124773,
	 0.8047954884
};

/*****************************0.3Hz_IIR__High_Filter****************************/
/*const float Gain = 0.99468273;
const float IIR_High_B[3] = {
	1,
	-2,
	1
};

const float IIR_High_A[3] = {
	1,
	-1.9833718117,
	0.9839372834
};*/
   
void filter(u16 *BUF)
{
	for (i=0;i<256;i++)
	{
		x0=(float)BUF[i]/4096*3.3;// 读取转换的AD值	

		w0[0]=IIR_50Notch_A[0] *x0-IIR_50Notch_A[1]*w0[1]-IIR_50Notch_A[2]*w0[2];
		y0=IIR_50Notch_B[0]*w0[0]+IIR_50Notch_B[1]*w0[1]+IIR_50Notch_B[2]*w0[2];

		/*w1[0]=IIR_High_A[0]*y0-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
		y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);*/
		IIR_Filter_Data[i]=y0;//缓存数组储存滤波之后的信号	
		
		w0[2]=w0[1];
		w0[1]=w0[0];
		/*w1[2]=w1[1];
		w1[1]=w1[0];*/
	}
	for(i=0;i<3;i++)
	{
		w0[i]=0;
		//w1[i]=0;		
	}
}

//LED配置，测试用
void LED_GPIOIni()
{
	   GPIO_InitTypeDef GPIO_InitStructure;
		 RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOD, ENABLE );
		 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		 GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/**
  * @brief  FATFS初始化函数
  * @param  无
  * @retval 无
  */
void FATFS_Init()
{
	  while(disk_initialize(0))                                                               //盘符0为sd卡
		{
			LCD_ShowString(60,150,200,16,16,"SD Card Error!");
			delay_ms(200);
			LCD_Fill(60,150,240,150+16,WHITE);//清除显示			  	
		};
		LCD_ShowString(120,150,200,16,16,"INITIAL OK!");
		delay_ms(800);
		LCD_Fill(120,150,240,150+16,WHITE);                                                     //清除显示	
		mem_init();                                                                             //初始化文件管理表
		exfuns_init();							                                                            //为fatfs相关变量申请内存			
  	f_mount(fs[0],"0:",1);                                                                  //挂载SD卡										 	
}

void mChange(float *fdata,u16 *BUF)
{
	for(i=0;i<256;i++)
	{
		BUF[i]=(u16)(fdata[i]*4096/3.3);	
	}
}	

void mSend(u16 *BUF)
{
    for(i=0;i<256;i++)
		{
			USART1->DR = ('@' & (uint16_t)0xFF);
			while( (USART1->SR & (uint16_t)0x0080)==(uint16_t)RESET );
								
			USART1->DR = ( (BUF[i]>>8)& (uint16_t)0xFF );										
			while( (USART1->SR & (uint16_t)0x0080)==(uint16_t)RESET );
			USART1->DR = (BUF[i]& (uint16_t)0xFF);										
			while( (USART1->SR & (uint16_t)0x0080)==(uint16_t)RESET );

			USART1->DR = ('|' & (uint16_t)0xFF);
			while( (USART1->SR & (u16)0x0080)==(uint16_t)RESET );
		}
}

/*void mL2C(u16 *BUF, u8 *ff)
{
	for(i=0;i<256;i++)
	{
		//u8 testbuf=(u8)BUF[i]>>8;
		ff[2*i]=(char)((BUF[i]>>8)& (uint16_t)0xFF);
		ff[2*i+1]=(char)(BUF[i]& (uint16_t)0xFF);
	}
}*/

/*直接用BUF1或BUF2可能会覆盖，最好对DMA flag判断使用哪个BUF*/
/*void mC2L(u16 *BUF,u8 *ff)
{
	for(i=0;i<256;i++)
	{
		BUF[i]=(((u16)ff[2*i]<<8)|(u16)ff[2*i+1]);	
	}
}*/
u16 a,k;
void mFirst_Draw()
{
	for(a=0;a<512;a++)
	{
		/*写数据进缓冲区*/
		if((a>=0)&&(a<=255))
		{	
			if(a==0) filter(BUF1);
			/*借用IIRDATA做运算*/
			//IIR_Filter_Data[i]=(float)BUF1[i]/4096*3.3;
			Draw_Data_int=(u16)((IIR_Filter_Data[a]-1.23)*50);
			write_int_data_LCD(Draw_Data_int);//往LCD绘图缓冲区写新值	
		}
		else if((a>255)&&(a<512))
		{
			if(a==256) filter(BUF2);
			/*借用IIRDATA做运算*/
			//IIR_Filter_Data[i-256]=(float)BUF2[i-256]/4096*3.3;
			Draw_Data_int=(u16)((IIR_Filter_Data[a-256]-1.23)*50);
			write_int_data_LCD(Draw_Data_int);//往LCD绘图缓冲区写新值	
		}
		 /*FFT频谱*/		
		if(i==0||i==320)
		{
			for(j=0;j<80;j++)
				LCD_Fill(4*j,0,4*(j+1)-1,(u16)(120*Mag[j]/Mag_total),BLACK);
			delay_ms(1000);
			for(j=0;j<80;j++)
				LCD_Fill(4*j,0,4*(j+1)-1,(u16)(120*Mag[j]/Mag_total),WHITE);		
		}		
		if(a<=319)
		{
			if(a==0)
				LCD_DrawPoint( a, LCD_DrawBuf[a] );//第一个点								
			else
				LCD_DrawLine( a-1, LCD_DrawBuf[a-1], a, LCD_DrawBuf[a] );
			delay_ms(2);
			if(a==319)
			{
				POINT_COLOR=WHITE;//画完清点
				for(k=1;k<320;k++)																		
				LCD_DrawLine( k-1, LCD_DrawBuf[k-1],k, LCD_DrawBuf[k] );
			  POINT_COLOR=BLUE;							
			}
		}
		else if((a>319)&&(a<512))
		{
			if((a+1)%64==0)
			{
				for(k=0;k<320;k++)
				{
					if(k==0)
						LCD_DrawPoint( k, LCD_DrawBuf[(a+1)>511?0:(a+1)] );//第一个点
					else
						LCD_DrawLine( k-1, LCD_DrawBuf[(a+k)>511?(a+k-512):(a+k)], k, LCD_DrawBuf[(a+k+1)>511?(a+k+1-512):(a+k+1)] );//j>1后的画点									
				}
				POINT_COLOR=WHITE;//画完清点
				for(k=0;k<320;k++)
				{
					if(k==0)
						LCD_DrawPoint( k, LCD_DrawBuf[(a+1)>511?0:(a+1)] );//第一个点
					else
						LCD_DrawLine( k-1, LCD_DrawBuf[(a+k)>511?(a+k-512):(a+k)], k, LCD_DrawBuf[(a+k+1)>511?(a+k+1-512):(a+k+1)] );//j>1后的画点
					delay_ms(1);									
				}
				POINT_COLOR=BLUE;
				/*R波检测*/				
				if(a==511)
				{
					Rprocess(LCD_DrawBuf);
					/*POINT_COLOR = RED;
					for(j=0;j<rnum;j++)										
						LCD_DrawRectangle(R[j].rx - 2, R[j].ry + 2, R[j].rx + 2, R[j].ry - 2);*/										
					rr_show = average_rr * 0.004;
					heart_rate = 60 / rr_show;
					LCD_ShowString(240,104,24,16,16,"HR:");
					LCD_ShowNum(270,104,(u32)heart_rate,3,16);
					//delay_ms(700); 
					/*POINT_COLOR=WHITE;//画完清点
					for(j=0;j<rnum;j++)																					
						LCD_DrawRectangle(R[j].rx - 2, R[j].ry + 2, R[j].rx + 2, R[j].ry - 2);*/											
					rnum=0;								
				}					
			}	
		}
	}
}

/*画图函数要改*/
void mElse_Draw()
{
	for(a=0;a<512;a++)
	{
		/*写数据进缓冲区*/
		if((a>=0)&&(a<=255))
		{	
			if(a==0) filter(BUF1);
			/*借用IIRDATA做运算*/
			//IIR_Filter_Data[i]=(float)BUF1[i]/4096*3.3;
			Draw_Data_int=(u16)((IIR_Filter_Data[a]-1.23)*50);
			write_int_data_LCD(Draw_Data_int);//往LCD绘图缓冲区写新值	
		}
		else if((a>255)&&(a<512))
		{
			if(a==256) filter(BUF2);
			/*借用IIRDATA做运算*/
			//IIR_Filter_Data[i-256]=(float)BUF2[i-256]/4096*3.3;
			Draw_Data_int=(u16)((IIR_Filter_Data[a-256]-1.23)*50);
			write_int_data_LCD(Draw_Data_int);//往LCD绘图缓冲区写新值	
		}
    /*FFT频谱*/		
		if(i==0||i==320)
		{
			for(j=0;j<80;j++)
				LCD_Fill(4*j,0,4*(j+1)-1,(u16)(120*Mag[j]/Mag_total),BLACK);
			delay_ms(1000);
			for(j=0;j<80;j++)
				LCD_Fill(4*j,0,4*(j+1)-1,(u16)(120*Mag[j]/Mag_total),WHITE);		
		}
		if((a+1)%64==0)
		{
			for(k=0;k<320;k++)
			{
				if(k==0)
					LCD_DrawPoint( k, LCD_DrawBuf[(a+1)>511?0:(i+1)] );//第一个点
				else
					LCD_DrawLine( k-1, LCD_DrawBuf[(a+k)>511?(a+k-512):(a+k)], k, LCD_DrawBuf[(a+k+1)>511?(a+k+1-512):(a+k+1)] );//j>1后的画点
				delay_ms(2);									
			}
			POINT_COLOR=WHITE;//画完清点
			for(k=0;k<320;k++)
			{
				if(k==0)
					LCD_DrawPoint( k, LCD_DrawBuf[(a+1)>511?0:(a+1)] );//第一个点
				else
					LCD_DrawLine( k-1, LCD_DrawBuf[(a+k)>511?(a+k-512):(a+k)], k, LCD_DrawBuf[(a+k+1)>511?(a+k+1-512):(a+k+1)] );//j>1后的画点												
			}
			POINT_COLOR=BLUE;
			/*R波检测*/				
			if(a==511)
			{
				Rprocess(LCD_DrawBuf);
				/*POINT_COLOR = RED;
				for(j=0;j<rnum;j++)										
					LCD_DrawRectangle(R[j].rx - 2, R[j].ry + 2, R[j].rx + 2, R[j].ry - 2);*/										
				rr_show = average_rr * 0.004;
				heart_rate = 60 / rr_show;
				LCD_ShowString(240,104,24,16,16,"HR:");
				LCD_ShowNum(270,104,(u32)heart_rate,3,16);
				//delay_ms(700); 
				//POINT_COLOR=WHITE;//画完清点
				/*for(j=0;j<rnum;j++)																					
					LCD_DrawRectangle(R[j].rx - 2, R[j].ry + 2, R[j].rx + 2, R[j].ry - 2);*/											
				rnum=0;								
			}					
		}					
	}
		
}	

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main ( void )
{		
		//u16 i;  
	  /* 初始化顺序要调 */
	  delay_init();                                                                      //初始化延时                                               
		LCD_Init();                                                                        //初始化LCD
		RTC_Init();                                                                        //初始化RTC
		LED_GPIOIni();                                                                     //初始化LED指示灯
    DMA_flag=BUF_Entrance;
	  ADCx_Init();                                                                       //初始化ADC
	  FATFS_Init();                                                                      //初始化文件系统
	  EXTIX_Init();                                                                      //初始化外部中断及按键，中断优先级要调
		if(Esp_Send_flag==1&&Sd_Store_flag==0&&Sd_Review_flag==0)
		{
			ESP8266_Init ();                                                                 //初始化WiFi模块使用的接口和外设
			macESP8266_Usart ( "+++" );                                                      //发送'+++'关闭透传
			ESP8266_APTcpClient_UnvarnishTest ();                                            //配置sta模式，开启透传		
		}

		/*to_tm(RTC_GetCounter(), &systmtime);//把定时器的值转换为北京时间
	  sprintf(Filname,"0:/%02d-%02d-%02d.txt",systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);
		POINT_COLOR=RED;
	  while((res=f_open(file,Filname,FA_WRITE|FA_CREATE_ALWAYS))!=FR_OK)
	  {		
		LCD_ShowString(60,150,200,16,16,"OPEN FAIL!");
		delay_ms(200);
		LCD_Fill(60,150,240,150+16,WHITE);//清除显示			
		}
	  for(i=0;i<256;i++)
	  {
			fatbuf[2*i]=0x40;
			fatbuf[2*i+1]=0x7C;			
		}
		bw=0;
		for(i=0;i<10;i++)
		{
			res=f_write(file,fatbuf,512,&bw);
			//f_sync(file);
			f_lseek(file,file->fsize);
      if(res!=FR_OK||bw<512)
      LCD_ShowString(60,150,200,16,16,"WRITE FAIL!");
      delay_ms(200);
			LCD_Fill(60,150,240,150+16,WHITE);//清除显示				
		}	
		f_close(file);					
    res=f_open(file,Filname,FA_READ|FA_OPEN_EXISTING);
		res=f_read(file,(u8*)BUF1,512,&br);
		f_close(file);*/
		while(1)
		{ 
      //发送模式			
			if(Esp_Send_flag==1&&Sd_Store_flag==0&&Sd_Review_flag==0)
			{
			/**********************************************************连接状态检测，掉线重连***********************************************************/
			//u8 TCP_FLAG=0; TCP_FLAG=ESP8266_Get_LinkStatus ();//获得连接状态
			//if(!TCP_FLAG)  
			//{
			//		while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );//断线重开TCP服务	
			//		while ( ! ESP8266_UnvarnishSend () );//重新进入透传
			//}				
				if(DMA_flag==BUF1_store_Finish)
				{
					filter(BUF1);
					mChange(IIR_Filter_Data,BUF1);            //发送前进行转换
					mSend(BUF1);
          					
					LCD_ShowString(60,220,64,16,16,"successA");
					mainflag1++;
					LCD_ShowNum(130, 220, mainflag1, 1, 16);										
				}
			  else if(DMA_flag==BUF2_store_Finish)
				{				
					filter(BUF2);
					mChange(IIR_Filter_Data,BUF2);            //发送前进行转换
					mSend(BUF2);
					
					LCD_ShowString(150,220,64,16,16,"successB");
					mainflag2++;
					LCD_ShowNum(230, 220, mainflag2, 1, 16);				
				}				

			}
			//储存模式
			if(Esp_Send_flag==0&&Sd_Store_flag==1&&Sd_Review_flag==0)
			{
				if(ID2==0)
				{
					to_tm(RTC_GetCounter(), &systmtime);/*把定时器的值转换为北京时间*/
	        sprintf(Filname,"0:/%02d-%02d-%02d.txt",systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);
					res=f_open(file,Filname,FA_WRITE|FA_CREATE_ALWAYS);	
					ID2=1;         					
				}				
				if(DMA_flag==BUF1_store_Finish)
				{
					LCD_ShowString(120,150,200,16,16,"Storing...");	
					filter(BUF1);
					mChange(IIR_Filter_Data,BUF1);            //写入前进行转换
					/*需要修改的：f_sync,或f_lseek，写完文件指针会不会变,如何确保关闭文件,对br、bw判断*/				
					res=f_write(file,(u8*)BUF1,512,&bw);
					f_sync(file);
					//f_lseek(file,file->fsize);
					
					LCD_Fill(120,150,240,150+16,WHITE);//清除显示	
					LCD_ShowString(60,220,64,16,16,"successA");
					mainflag1++;
					LCD_ShowNum(130, 220, mainflag1, 1, 16);					
					//delay_ms(1000);					
					/*DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //重新使能DMA
					ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求！！！*/
					DMA_flag=BUF1_filter_Finish;	
				}
			  else if(DMA_flag==BUF2_store_Finish)
				{
					LCD_ShowString(120,150,200,16,16,"Storing...");	
					filter(BUF2);
					mChange(IIR_Filter_Data,BUF2);            //写入前进行转换									
					res=f_write(file,(u8*)BUF2,512,&bw);
					f_sync(file);
										
					LCD_Fill(120,150,240,150+16,WHITE);//清除显示						
					LCD_ShowString(150,220,64,16,16,"successB");
					mainflag2++;
					LCD_ShowNum(230, 220, mainflag2, 1, 16);
					//delay_ms(1000);						
					/*DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //重新使能DMA
          ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求！！！*/
			    DMA_flag=BUF2_filter_Finish;						
				}
			}
			//回放模式
			if(Esp_Send_flag==0&&Sd_Store_flag==0&&Sd_Review_flag==1)
			{
				/*****************************************************************绘制坐标系*****************************************************************/
				POINT_COLOR=BRED;
				LCD_DrawLine(0,120,319,120);	
				POINT_COLOR=BLUE;
				
				/*读取文件时用到,注意文件空时的处理，用drawbuf代替？*/
				if(ID1==0)//第一次画全屏，仅用于第一次BUF1绘制
				{
					res=f_open(file,Filname,FA_READ|FA_OPEN_EXISTING);
					while(res!=FR_OK)
					{
							LCD_ShowString(60,150,200,16,16,"read fail!");
							delay_ms(200);
							LCD_Fill(60,150,240,150+16,WHITE);//清除显示							
					}					
					ID1=1;							
				}
				LCD_ShowString(120,150,200,16,16,"Replaying...");					
				res=f_read(file,(u8*)BUF1,512,&br);
				f_sync(file);				
				FFT(BUF1);
				if(f_tell(file)>=(file->fsize))
				{
					Sd_Review_flag=0;
					Esp_Send_flag=1;
					f_close(file);
				  LCD_ShowString(120,150,200,16,16,"Returning...");	
          delay_ms(200);
					LCD_Fill(60,150,240,150+16,WHITE);//清除显示
					DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值
					DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //重新使能DMA
					ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求！！！					
				}				
				res=f_read(file,(u8*)BUF2,512,&br);
				f_sync(file);
				FFT(BUF2);
				if(f_tell(file)>=(file->fsize))
				{
					Sd_Review_flag=0;
					Esp_Send_flag=1;
					f_close(file);
				  LCD_ShowString(120,150,200,16,16,"Returning...");	
          delay_ms(200);
					LCD_Fill(60,150,240,150+16,WHITE);//清除显示
					DMA_SetCurrDataCounter(DMA1_Channel1,256);//重新设置当前数据计数器值
					DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);     //重新使能DMA
					ADC_DMACmd(macADCx, ENABLE);              //重新使能ADC DMA 请求！！！					
				}				
				/************************************LCD流动画图**********************************/
				if(Sd_Review_flag==1)//再次检测
				{
					if(ID==0)//第一次画全屏，仅用于第一次BUF1绘制
					{
						mFirst_Draw();
						ID=1;							
					}              						
					else if(ID==1)//第二次以后流动画图
					{
						mElse_Draw();						
					}
					LCD_Fill(120,150,240,150+16,WHITE);//清除显示								
				}
					
			}
				
		}	       
			
}



/*********************************************END OF FILE**************************************************/
