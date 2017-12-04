/**
  ******************************************************************************
  * @file    Rsearch.c
  * @author  Syaru
  * @version V1.0
  * @date    2016-08-17
  * @brief   һ�ֲ��������ֵ�ж������ҽ��ж��ļ�ֵ�㴢����һ�����������б��ڿ���
	�����뻭ͼ	
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
	/*********************��ּ���*********************/
	for (i = 1; i < 511; i++) //������һ������һ��
	{		
		chaf[i] = ecg[i + 1] - ecg[i - 1];//������
		//if(chaf[i]>CfGate) CfGate=chaf[i];
		if(ecg[i]>EcgGate) EcgGate=ecg[i];
    	
	}
	CfGate=0.5*CfGate;
	EcgGate=0.8*EcgGate;
	/********************��ֵ�ж�**********************/
	for (i = 6; i < 506; i++)//�ҵ���ֵ��//chef[0]/chef[319]������,��Χ6~313
	{
		flag1=0;
		flag2=0;
		for (j = -5; j <0; j++)//������u16����
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
	/******************R���ж�����¼*******************/
	for (i = 6; i < 506; i++) 
	{   
		if (rlabel[i] == 1) 
		{
			maxflag1 = 1;
			maxflag2 = 1;
			/*��߽粻��15��*/
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
			/*�ұ߽粻��15��*/
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
			/*��������*/
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
		/*���˴���R��*/
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
		/*��Ч��R��*/
		if (rlabel[i] == 1) 
		{			
			/*�ṹ��*/			
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
				/*R-R����¼*/
				rr = i - r_valid;
				r_valid = i;
				total_rr += rr;
			}
			rnum++;

		}
		
	}
	/***************����ƽ��R-Rֵ**************/
	average_rr = (float)total_rr / rnum;//average_rrΪȫ�ֱ���
}


