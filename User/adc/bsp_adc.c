#include "bsp_adc.h"

__IO u8 DMA_flag;     //DMA标志位
__IO u16 BUF1[256]={0};  //缓冲1区
__IO u16 BUF2[256]={0};  //缓冲2区

/**
  * @brief  ADC GPIO 初始化
  * @param  无
  * @retval 无
  */
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 打开 ADC IO端口时钟
	macADC_GPIO_APBxClock_FUN ( macADC_GPIO_CLK, ENABLE );
	
	// 配置 ADC IO 引脚模式
	GPIO_InitStructure.GPIO_Pin = macADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	// 初始化 ADC IO
	GPIO_Init(macADC_PORT, &GPIO_InitStructure);				
}

/**
  * @brief  配置ADC工作模式
  * @param  无
  * @retval 无
  */
static void ADCx_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 配置DMA中断 */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//8266 USART配置时已经设置
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	// 打开DMA时钟
	macADC_APBxClock_FUN ( macADC_CLK, ENABLE );	// 打开ADC时钟
			
	/* 配置 DMA 初始化结构体 */
	// 复位DMA控制器
	DMA_DeInit(macADC_DMA_CHANNEL);
	// 外设基址为：ADC 数据寄存器地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 ) ( & ( macADCx->DR ) );
	// 存储器地址，实际上就是一个内部SRAM的变量
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BUF1;	
	// 数据源来自外设
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	
	// 只传输一个数据(count	寄存器)
	DMA_InitStructure.DMA_BufferSize = 256;	
	// 外设寄存器只有一个，地址不用递增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// 存储器地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 	
	// 外设数据大小为半字，即两个字节
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	
	// 内存数据大小也为半字，跟外设数据大小相同
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// 循环传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	// DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	
	// 禁止存储器到存储器模式，因为是从外设到存储器
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	
	// 初始化DMA
	DMA_Init(macADC_DMA_CHANNEL, &DMA_InitStructure);
	// 使能 DMA 通道
	DMA_Cmd(macADC_DMA_CHANNEL , ENABLE);
	//清除全部中断标志
	DMA_ClearITPendingBit(DMA1_IT_GL1);
	//允许TC中断
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);	


	/* ADC 模式配置 */
	ADC_DeInit(macADCx);
	// 只使用一个ADC，属于单模式
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	// 禁止扫描模式，多通道才要，单通道不需要
	ADC_InitStructure.ADC_ScanConvMode = DISABLE ; 
	// 禁止连续转换模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		
	// 使用外部触发，不使用软件触发!!!!
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;		
	// 转换结果右对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	// 转换通道1个
	ADC_InitStructure.ADC_NbrOfChannel = 1;			
	// 初始化ADC
	ADC_Init(macADCx, &ADC_InitStructure);


	// 配置ADC时钟为PCLK2的6分频，即12MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	// 配置 ADC 通道转换顺序为1，第一个转换，采样时间+转换时间为239.5+12.5=252个时钟周期
	ADC_RegularChannelConfig(macADCx, macADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);		
	// 开启ADC ，并开始转换( ADON=1时才能进行校准 )
	ADC_Cmd(macADCx, ENABLE);
	
	// 初始化ADC 校准寄存器  
	ADC_ResetCalibration(macADCx);
	// 等待校准寄存器初始化完成
	while(ADC_GetResetCalibrationStatus(macADCx));
	// ADC开始校准
	ADC_StartCalibration(macADCx);
	// 等待校准完成
	while(ADC_GetCalibrationStatus(macADCx));	 
	// 使能ADC DMA 请求,有效时间normal模式下为一次dma循环？
	ADC_DMACmd(macADCx, ENABLE);

	 // 使能外部触发!!!!
	ADC_ExternalTrigConvCmd(macADCx,ENABLE);	
	// 由于没有采用外部触发，所以使用软件触发ADC转换 
	//ADC_SoftwareStartConvCmd(macADCx, ENABLE);
	
	/*************************等待定时器触发***************************/
}

/*void TIM_init()
{
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_OCInitTypeDef         TIM_OCInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);   //开启定时器时钟
	
	// Time base configuration
	TIM_TimeBaseStructure.TIM_Period = 5000-1;      //当定时器从0计数4999，即为5000次，为一个定时周期
  TIM_TimeBaseStructure.TIM_Prescaler = 72-1;   //系统主频72M，这里分频72，相当于100K的定时器2时钟
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);
	
	// PWM Mode configuration: Channel2 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//下面详细说明
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable; 
	TIM_OCInitStructure.TIM_Pulse = 2500; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//当定时器计数值小于CCR1_Val时为高电平
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); //配置通道2
	
	// 使能定时器2 
	TIM_Cmd(TIM2, ENABLE); 
	TIM_InternalClockConfig(TIM2); 
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable); //使能通道2
	TIM_UpdateDisableConfig(TIM2, DISABLE); 
}*/

void TIM3_Init()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
  //打开timer3 gpio？af没有用到输出功能不需要重映射
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
	
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);//选择TRGO触发源为计时器更新时间
	//TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);
	//LCD_ShowNum(60,0,4,1,16);
	//TIM_ClearFlag(TIM3,TIM_FLAG_Update);    //清除配置产生的中断标志
	//TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//允许更新中断	

  TIM_Cmd(TIM3,ENABLE); 
}

/**
  * @brief  ADC/DMA/TIMER初始化
  * @param  无
  * @retval 无
  */
void ADCx_Init(void)
{
	ADCx_GPIO_Config();
	ADCx_Mode_Config();
	TIM3_Init();	
}
/*********************************************END OF FILE**********************/
