#include "main.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include "stm32l4xx_it.h"

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

int flag;
int reading;
	//uint32_t vref;
int temp;

int UART_Print_String(UART_HandleTypeDef *huart1, char *string, int len);
int UART_Print_String_DMA(UART_HandleTypeDef *huart1, char *string, int len);
void SystemClock_Config(void);
static void MX_ADC_Init(void);
static void MX_ADC_Calib(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA_Init(void);

int main(void)
{
	char ch[5] = {'j','o','b','s','\n'};
	char s[50];
	uint32_t reading;
	//uint32_t vref;
	uint32_t temp;
	char temp_array[30];
	int i =0;
	char t[3];

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
	//init dma
	MX_DMA_Init();
	
	/* Initialize and calibrate ADC */
	MX_ADC_Init();
	MX_ADC_Calib();
	
	//Following the steps from the HAL drivers manual p. 104: start -> poll -> get value
	
	
	//int32_t cal2 = (int32_t) *TEMPSENSOR_CAL2_ADDR;
	//int32_t cal1 = (int32_t) *TEMPSENSOR_CAL1_ADDR;
	
	/*
	*Already done when calling transmit
	uint32_t srcAddress =(uint32_t ) &temp_array;
	uint32_t dstAddress = (uint32_t) &(GPIOC->ODR);
	HAL_DMA_Start_IT(&hdma_usart1_tx,srcAddress, dstAddress,30); //DATA LENGTH WRONG
	*/
	
  /* Infinite loop */
  while (1)
  {
		//HAL_Delay(100);
		//HAL_UART_Transmit(&huart1, (uint8_t *)&ch[0], 5, 30000);
		
		if(flag == 1)
		{
			flag =0;
			HAL_ADC_Start(&hadc1);
			
			if(HAL_ADC_PollForConversion(&hadc1, 30000)== HAL_OK)
			{
				reading = HAL_ADC_GetValue(&hadc1);								//Get current temperature
				temp = __HAL_ADC_CALC_TEMPERATURE(3311, reading, ADC_RESOLUTION_10B);
				
				/*
				sprintf(s, "Temperature = %d C\n", temp);				//Convert from int to string
				int len = strlen(s);
				UART_Print_String(&huart1, s, len);
				*/
				
				if(i<30){
					sprintf(t,"%d\n",temp);
					temp_array[i]=t[0];
					temp_array[i+1]=t[1];
					temp_array[i+2]=t[2];
					i+=3;
				}
				else{
					i=0;
					int len = 30;
					//UART_Print_String_DMA(&huart1, temp_array, len);
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) &temp_array,len);
				}
			}	
			HAL_ADC_Stop(&hadc1);
		
																									//Wait for conversion
			//reading = HAL_ADC_GetValue(&hadc1);								//Get current temperature
			
			/**  
			//To compute Analog reference voltage (Vref+), use following macro (ADC channel has to be changed from
			//ADC_CHANNEL_TEMPSENSOR to ADC_CHANNEL_VREFINT in MX_ADC_Init(). FOUND THAT VREF = 3311 mV and not 3000 mV
			vref = __LL_ADC_CALC_VREFANALOG_VOLTAGE(reading, ADC_RESOLUTION_12B);
			sprintf(s, "vref = %d C\n", vref);				//Convert from int to string
			int len = strlen(s);
			UART_Print_String(&huart1, s, len);
			*/
		}
  }
}

/* Transmit string through UART */
int UART_Print_String(UART_HandleTypeDef *huart1, char *string, int len) {
		if (HAL_UART_Transmit(huart1, (uint8_t *)&string[0], len, 30000) == HAL_OK)
			return 1;	
		return 0;
}
int UART_Print_String_DMA(UART_HandleTypeDef *huart1, char *string, int len) {
		if(HAL_UART_Transmit_DMA(huart1,(uint8_t *)&string[0],len) == HAL_OK){
			
			return 1;
		}
		return 0;
}

/* ADC init function (following configuration from Tutorial 4)*/
void MX_ADC_Init(void) {
	
	ADC_ChannelConfTypeDef chConfig;
	
	__HAL_RCC_ADC_CLK_ENABLE();
	
	hadc1.Instance = ADC1;
	
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_10B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.NbrOfDiscConversion = 0;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_CC1;//ADC_SOFTWARE_START
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	//hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = DISABLE;
	
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		 _Error_Handler(__FILE__, __LINE__);
	}
	
	chConfig.Channel = ADC_CHANNEL_TEMPSENSOR;				//From MCU datasheet p.36 and ref. manual p.580, temp. sensor is internally 
																										//connected to channel 17. Only available on ADC1 and ADC3 instances
	chConfig.Rank = 1;
	chConfig.SamplingTime = LL_ADC_SAMPLINGTIME_47CYCLES_5;
	chConfig.Offset = 0;
	
	if (HAL_ADC_ConfigChannel(&hadc1, &chConfig) != HAL_OK)
	{
		 _Error_Handler(__FILE__, __LINE__);
	}

}

/* ADC Self-calibration */
static void MX_ADC_Calib(void) {
	
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)  
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 32;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/10);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{
	
	__USART1_CLK_ENABLE();//TEST
	
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	/*
	  *          This parameter can be one of the following values:
  *            @arg @ref UART_IT_RXFF  RXFIFO Full interrupt
  *            @arg @ref UART_IT_TXFE  TXFIFO Empty interrupt
  *            @arg @ref UART_IT_RXFT  RXFIFO threshold interrupt
  *            @arg @ref UART_IT_TXFT  TXFIFO threshold interrupt
  *            @arg @ref UART_IT_WUF   Wakeup from stop mode interrupt
  *            @arg @ref UART_IT_CM    Character match interrupt
  *            @arg @ref UART_IT_CTS   CTS change interrupt
  *            @arg @ref UART_IT_LBD   LIN Break detection interrupt
  *            @arg @ref UART_IT_TXE   Transmit Data Register empty interrupt
  *            @arg @ref UART_IT_TXFNF TX FIFO not full interrupt
  *            @arg @ref UART_IT_TC    Transmission complete interrupt
  *            @arg @ref UART_IT_RXNE  Receive Data register not empty interrupt
  *            @arg @ref UART_IT_RXFNE RXFIFO not empty interrupt
  *            @arg @ref UART_IT_IDLE  Idle line detection interrupt
  *            @arg @ref UART_IT_PE    Parity Error interrupt
  *            @arg @ref UART_IT_ERR   Error interrupt (Frame error, noise error, overrun error)
	*/
	//__HAL_UART_ENABLE_IT(&huart1,UART_IT_TC);
	  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
	
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();


	
	//user code
	/*
			2. For a given Channel, program the required configuration through the following
					parameters: Channel request, Transfer Direction, Source and Destination data
					formats, Circular or Normal mode, Channel Priority level, Source and Destination
					Increment mode using HAL_DMA_Init() function. Prior to HAL_DMA_Init the
					peripheral clock shall be enabled for both DMA & DMAMUX thanks to:
					a. DMA1 or DMA2: __HAL_RCC_DMA1_CLK_ENABLE() or
					__HAL_RCC_DMA2_CLK_ENABLE() ;
					b. DMAMUX1: __HAL_RCC_DMAMUX1_CLK_ENABLE();
	
	*/
	
	hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_usart1_tx.Init.Mode = DMA_NORMAL;//NORMAL???
	hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_usart1_tx.Init.MemInc= DMA_MINC_ENABLE;
	hdma_usart1_tx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
	hdma_usart1_tx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
	hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	
	if(HAL_DMA_Init(&hdma_usart1_tx)!=HAL_OK)
	{
    _Error_Handler(__FILE__, __LINE__);
  }	
	
	__HAL_LINKDMA(&huart1,hdmatx,hdma_usart1_tx);
	
	/* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration *///USART
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	

	
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
}


void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
}
#endif /* USE_FULL_ASSERT */
