#include "test.h"
#include "bsp_esp8266.h"
//#include "bsp_SysTick.h"
#include <stdio.h>  
#include <string.h>  
#include <stdbool.h>
#include "lcd.h"  

volatile uint8_t ucTcpClosedFlag = 0;
char *port1="49654";
char *port2="54370";
char *port3="2016";

/**
  * @brief  ESP8266 （Sta Tcp Client）透传
  * @param  无
  * @retval 无
  */

void ESP8266_APTcpClient_UnvarnishTest(void)
{

	ESP8266_AT_Test ();
	LCD_ShowNum(0, 0, 1, 1, 16);
	
	/*ESP8266_StartOrShutServer(ENABLE,"333","1");
	LCD_ShowNum(40, 0, 3, 1, 16);*/
	
	ESP8266_Net_Mode_Choose ( AP );
	LCD_ShowNum(20, 0, 2, 1, 16);
	
  ESP8266_Cmd ( "AT+RST", "OK", "ready", 1000 );
  LCD_ShowNum(40, 0, 3, 1, 16);	
	
	ESP8266_Enable_MultipleId ( DISABLE );
	//自动重连
	while(1)
	{
		/*if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port1, Single_ID_0 )) 
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;
		}
		if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port1, Single_ID_0 )) 
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;
		}*/
		if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port2, Single_ID_0 ))
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;		
		}	
		if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port2, Single_ID_0 ))
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;		
		}	
		/*if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port3, Single_ID_0 ))
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;		
		}
		if(ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, port3, Single_ID_0 ))
		{
			 LCD_Fill(60,60,240,150+16,WHITE);//清除显示			
		   break;		
		}*/
	  LCD_ShowString(60,60,200,16,16,"CONNECT FAIL!");		
	}
	
	while ( ! ESP8266_UnvarnishSend () );
	LCD_ShowNum(80, 0, 5, 1, 16);

}


void ESP8266_StaTcpClient_UnvarnishTest ( void )
{

	
	//char cStr [ 100 ] = { 0 };
	
	//DHT11_Data_TypeDef DHT11_Data;	
  //printf ( "\r\n正在配置 ESP8266 ......\r\n" );

	//macESP8266_CH_ENABLE();
	//USART_InitTypeDef USART_InitStructure;
	//USART_InitStructure.USART_BaudRate = 256000;
	//USART_Init(macESP8266_USARTx, &USART_InitStructure);
	
  //while(!ESP8266_Cmd ( "AT+UART=256000,8,1,0,0", "OK", 0, 500 ));
	//LCD_ShowNum(100, 0, 8, 1, 16);*/
	
	ESP8266_AT_Test ();
	LCD_ShowNum(0, 0, 1, 1, 16);
	
	ESP8266_Net_Mode_Choose ( STA );
	LCD_ShowNum(20, 0, 2, 1, 16);

  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	LCD_ShowNum(40, 0, 3, 1, 16);
	
	ESP8266_Enable_MultipleId ( DISABLE );
	
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	LCD_ShowNum(60, 0, 4, 1, 16);
	
	while ( ! ESP8266_UnvarnishSend () );
	LCD_ShowNum(80, 0, 5, 1, 16);
	
		
	//printf ( "\r\n配置 ESP8266 完毕\r\n" );
			
}

//检测是否失去连接,在while(1)中调用
void ESP8266_Connect_Test(void)
{    		
		uint8_t ucStatus;
	  if ( ucTcpClosedFlag )                                             
		{
			ESP8266_ExitUnvarnishSend ();                                    //退出透传模式				
			do ucStatus = ESP8266_Get_LinkStatus ();                         //获取连接状态
			while ( ! ucStatus );				
			if ( ucStatus == 4 )                                             //确认失去连接后重连
			{
				//printf ( "\r\n正在重连热点和服务器 ......\r\n" );					
				while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );					
				while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );					
				//printf ( "\r\n重连热点和服务器成功\r\n" );
			}				
			while ( ! ESP8266_UnvarnishSend () );					
		}
}


