#ifndef  __TEST_H
#define	 __TEST_H



#include "stm32f10x.h"



/********************************** �û���Ҫ���õĲ���**********************************/
//Ҫ���ӵ��ȵ�����ƣ���WIFI����
#define      macUser_ESP8266_ApSsid           "sern" 

//Ҫ���ӵ��ȵ����Կ
#define      macUser_ESP8266_ApPwd            "123080411" 

//Ҫ���ӵķ������� IP�������Ե�IP
#define      macUser_ESP8266_TcpServer_IP     "192.168.4.2" 

//Ҫ���ӵķ������Ķ˿�
#define      macUser_ESP8266_TcpServer_Port    "8735"         



/********************************** �ⲿȫ�ֱ��� ***************************************/
extern volatile uint8_t ucTcpClosedFlag;



/********************************** ���Ժ������� ***************************************/
void   ESP8266_StaTcpClient_UnvarnishTest  ( void );
void   ESP8266_Connect_Test  ( void );
void   ESP8266_APTcpClient_UnvarnishTest(void);

#endif

