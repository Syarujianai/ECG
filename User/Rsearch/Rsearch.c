/**
  ******************************************************************************
  * @file    Rsearch.c
  * @author  Syaru
  * @version V1.0
  * @date    2016-08-17
  * @brief   一种差分正负极值判定法，且将判定的极值点储存在一个单向链表中便于快速
	查找与画图	
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
#include "Rsearch.h"
//#include "malloc.h"
//#include <stdlib.h>

u16 rlabel[512] = { 0 }, r_valid = 0, EcgGate=0 ,Ecgmax=0;
u8  rnum = 0;
int chaf[512], tempecg, CfGate=0, Cfmax=0;
float average_rr;
RSTORE R[20];	

void Rprocess(u16 *ecg)
{
	u8 flag1=0, flag2=0, maxflag1=1, maxflag2=1;
	u16 total_ecg = 0, average_ecg = 0;
	u16 rr = 0, total_rr = 0;
	int i,j;
	/*********************差分计算*********************/
	for (i = 1; i < 511; i++) //舍弃第一点和最后一点
	{		
		chaf[i] = ecg[i + 1] - ecg[i - 1];//隔点差分
		//if(chaf[i]>CfGate) CfGate=chaf[i];
		if(ecg[i]>EcgGate) EcgGate=ecg[i];
    	
	}
	CfGate=0.5*CfGate;
	EcgGate=0.8*EcgGate;
	/********************极值判定**********************/
	for (i = 6; i < 506; i++)//找到极值点//chef[0]/chef[319]不存在,范围6~313
	{
		flag1=0;
		flag2=0;
		for (j = -5; j <0; j++)//不能用u16类型
		{
			if (chaf[i + j] < 0)
				break;
			if (j == -1)
				flag1 = 1;
		}

		for (j = 1; j <= 5; j++)
		{
			if (chaf[i + j] > 0)
				break;
			if (j == 5)
				flag2 = 1;
		}

		if (flag1&&flag2) 
			rlabel[i] = 1;
	}
	/******************R波判定及记录*******************/
	for (i = 6; i < 506; i++) 
	{   
		if (rlabel[i] == 1) 
		{
			maxflag1 = 1;
			maxflag2 = 1;
			/*左边界不足15点*/
			if (i < 20) 
			{
				for (j = 1; j <= 20; j++) 
					if (ecg[i + j] > ecg[i])
					{
						maxflag1 = 0;
						break;
					}
				if (maxflag1 == 0) continue;					
							
				for (j = 0; j < i; j++) 				
					if (ecg[j] > ecg[i])
					{
						maxflag2 = 0;
						break;
					}
				if (maxflag1&&maxflag2 == 0)
					rlabel[i] = 0;
			}
			/*右边界不足15点*/
			else if (i > 496) 
			{
				for (j = -20; j < 0; j++) 
					if (ecg[i + j] > ecg[i]) 
					{
						maxflag1 = 0;
						break;
					}
				if (maxflag1 == 0) continue;
				
				for (j = 511; j > i; j--)
					if (ecg[j] > ecg[i])
					{
						maxflag2 = 0;
						break;
					}
				if (maxflag1&&maxflag2 == 0)
					rlabel[i] = 0;
			}
			/*正常区域*/
			else 
			{
				for (j = 1; j <= 20; j++)
					if (ecg[i + j] > ecg[i])
					{
						maxflag1 = 0;
						break;
					}
				if (maxflag1 == 0) continue;

				for (j = -20; j < 0; j++)
					if (ecg[i + j] > ecg[i])
					{
						maxflag2 = 0;
						break;
					}
				if (maxflag1&&maxflag2 == 0)
					rlabel[i] = 0;
			}

		}
		/////////////////
		/*if(rlabel[i] == 1)
		{
			total_ecg+=ecg[i];
			num++;		
		}*/
	}
  //average_ecg=total_ecg / num;
	for(i = 6; i < 506; i++)
	{
		/*过滤错误R波*/
		/*if(rlabel[i] == 1)
			if(ecg[i]<average_ecg)*/
				rlabel[i] = 0;
		for(j=-20;j<i;j++)
		{
			if(chaf[i]>Cfmax)
			  Cfmax=chaf[i];
			if(ecg[i]>Ecgmax)
				Ecgmax=ecg[i];
		}
		if(Ecgmax<EcgGate)
			rlabel[i]=0;			
		/*有效的R波*/
		if (rlabel[i] == 1) 
		{			
			/*结构体*/			
			if (rnum == 0)			
			{	
				R[rnum].rx=i;
				R[rnum].ry=ecg[i];
				r_valid = i;
			}	
			else if(rnum > 0)
			{
				R[rnum].rx=i;
				R[rnum].ry=ecg[i];
				/*R-R间距记录*/
				rr = i - r_valid;
				r_valid = i;
				total_rr += rr;
			}
			rnum++;

		}
		
	}
	/***************计算平均R-R值**************/
	average_rr = (float)total_rr / rnum;//average_rr为全局变量
}


