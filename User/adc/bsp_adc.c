#include "bsp_adc.h"

__IO u8 DMA_flag;     //DMA��־λ
__IO u16 BUF1[256]={0};  //����1��
__IO u16 BUF2[256]={0};  //����2��

/**
  * @brief  ADC GPIO ��ʼ��
  * @param  ��
  * @retval ��
  */
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// �� ADC IO�˿�ʱ��
	macADC_GPIO_APBxClock_FUN ( macADC_GPIO_CLK, ENABLE );
	
	// ���� ADC IO ����ģʽ
	GPIO_InitStructure.GPIO_Pin = macADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	// ��ʼ�� ADC IO
	GPIO_Init(macADC_PORT, &GPIO_InitStructure);				
}

/**
  * @brief  ����ADC����ģʽ
  * @param  ��
  * @retval ��
  */
static void ADCx_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* ����DMA�ж� */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//8266 USART����ʱ�Ѿ�����
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	// ��DMAʱ��
	macADC_APBxClock_FUN ( macADC_CLK, ENABLE );	// ��ADCʱ��
			
	/* ���� DMA ��ʼ���ṹ�� */
	// ��λDMA������
	DMA_DeInit(macADC_DMA_CHANNEL);
	// �����ַΪ��ADC ���ݼĴ�����ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 ) ( & ( macADCx->DR ) );
	// �洢����ַ��ʵ���Ͼ���һ���ڲ�SRAM�ı���
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BUF1;	
	// ����Դ��������
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	
	// ֻ����һ������(count	�Ĵ���)
	DMA_InitStructure.DMA_BufferSize = 256;	
	// ����Ĵ���ֻ��һ������ַ���õ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// �洢����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 	
	// �������ݴ�СΪ���֣��������ֽ�
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	
	// �ڴ����ݴ�СҲΪ���֣����������ݴ�С��ͬ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// ѭ������ģʽ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	// DMA ����ͨ�����ȼ�Ϊ�ߣ���ʹ��һ��DMAͨ��ʱ�����ȼ����ò�Ӱ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	
	// ��ֹ�洢�����洢��ģʽ����Ϊ�Ǵ����赽�洢��
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	
	// ��ʼ��DMA
	DMA_Init(macADC_DMA_CHANNEL, &DMA_InitStructure);
	// ʹ�� DMA ͨ��
	DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);
	//���ȫ���жϱ�־
	DMA_ClearITPendingBit(DMA1_IT_GL1);
	//����TC�ж�
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);	


	/* ADC ģʽ���� */
	ADC_DeInit(macADCx);
	// ֻʹ��һ��ADC�����ڵ�ģʽ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	// ��ֹɨ��ģʽ����ͨ����Ҫ����ͨ������Ҫ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE ; 
	// ��ֹ����ת��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		
	// ʹ���ⲿ��������ʹ���������!!!!
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;		
	// ת������Ҷ���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	// ת��ͨ��1��
	ADC_InitStructure.ADC_NbrOfChannel = 1;			
	// ��ʼ��ADC
	ADC_Init(macADCx, &ADC_InitStructure);


	// ����ADCʱ��ΪPCLK2��6��Ƶ����12MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	// ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��+ת��ʱ��Ϊ239.5+12.5=252��ʱ������
	ADC_RegularChannelConfig(macADCx, macADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);		
	// ����ADC ������ʼת��( ADON=1ʱ���ܽ���У׼ )
	ADC_Cmd(macADCx, ENABLE);
	
	// ��ʼ��ADC У׼�Ĵ���  
	ADC_ResetCalibration(macADCx);
	// �ȴ�У׼�Ĵ�����ʼ�����
	while(ADC_GetResetCalibrationStatus(macADCx));
	// ADC��ʼУ׼
	ADC_StartCalibration(macADCx);
	// �ȴ�У׼���
	while(ADC_GetCalibrationStatus(macADCx));	 
	// ʹ��ADC DMA ����,��Чʱ��normalģʽ��Ϊһ��dmaѭ����
	ADC_DMACmd(macADCx, ENABLE);

	 // ʹ���ⲿ����!!!!
	ADC_ExternalTrigConvCmd(macADCx,ENABLE);	
	// ����û�в����ⲿ����������ʹ���������ADCת�� 
	//ADC_SoftwareStartConvCmd(macADCx, ENABLE);
	
	/*************************�ȴ���ʱ������***************************/
}

/*void TIM_init()
{
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_OCInitTypeDef         TIM_OCInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);   //������ʱ��ʱ��
	
	// Time base configuration
	TIM_TimeBaseStructure.TIM_Period = 5000-1;      //����ʱ����0����4999����Ϊ5000�Σ�Ϊһ����ʱ����
  TIM_TimeBaseStructure.TIM_Prescaler = 72-1;   //ϵͳ��Ƶ72M�������Ƶ72���൱��100K�Ķ�ʱ��2ʱ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);
	
	// PWM Mode configuration: Channel2 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//������ϸ˵��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable; 
	TIM_OCInitStructure.TIM_Pulse = 2500; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); //����ͨ��2
	
	// ʹ�ܶ�ʱ��2 
	TIM_Cmd(TIM2, ENABLE); 
	TIM_InternalClockConfig(TIM2); 
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable); //ʹ��ͨ��2
	TIM_UpdateDisableConfig(TIM2, DISABLE); 
}*/

void TIM3_Init()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
  //��timer3 gpio��afû���õ�������ܲ���Ҫ��ӳ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
  
	//LCD_ShowNum(80,0,6,1,16);	
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	//LCD_ShowNum(40,0,3,1,16);
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 400-1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 720-1;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
	
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);//ѡ��TRGO����ԴΪ��ʱ������ʱ��
	//TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);
	//LCD_ShowNum(60,0,4,1,16);
	//TIM_ClearFlag(TIM3,TIM_FLAG_Update);    //������ò������жϱ�־
	//TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//��������ж�	

  TIM_Cmd(TIM3,ENABLE); 
}

/**
  * @brief  ADC/DMA/TIMER��ʼ��
  * @param  ��
  * @retval ��
  */
void ADCx_Init(void)
{
	ADCx_GPIO_Config();
	ADCx_Mode_Config();
	TIM3_Init();	
}
/*********************************************END OF FILE**********************/
