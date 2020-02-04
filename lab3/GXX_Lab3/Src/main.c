#include "main.h"
#include "stm32l4xx_hal.h"
#include <string.h>

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

int flag;
int reading;
	//uint32_t vref;
int temp;

int UART_Print_String(UART_HandleTypeDef *huart1, char *string, int len);
void SystemClock_Config(void);
static void MX_ADC_Init(void);
static void MX_ADC_Calib(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
	char ch[5] = {'j','o','b','s','\n'};
	char s[50];
	uint32_t reading;
	//uint32_t vref;
	uint32_t temp;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
	
	/* Initialize and calibrate ADC */
	MX_ADC_Init();
	MX_ADC_Calib();
	
	//Following the steps from the HAL drivers manual p. 104: start -> poll -> get value
	
	
	//int32_t cal2 = (int32_t) *TEMPSENSOR_CAL2_ADDR;
	//int32_t cal1 = (int32_t) *TEMPSENSOR_CAL1_ADDR;

  /* Infinite loop */
  while (1)
  {
		HAL_Delay(100);
		//HAL_UART_Transmit(&huart1, (uint8_t *)&ch[0], 5, 30000);
		if(flag == 1)
		{
			flag =0;
			HAL_ADC_Start(&hadc1);
			if(HAL_ADC_PollForConversion(&hadc1, 30000)== HAL_OK)
			{
				reading = HAL_ADC_GetValue(&hadc1);								//Get current temperature
				temp = __HAL_ADC_CALC_TEMPERATURE(3311, reading, ADC_RESOLUTION_10B);
			
				sprintf(s, "Temperature = %d C\n", temp);				//Convert from int to string
				int len = strlen(s);
				UART_Print_String(&huart1, s, len);
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
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_CC1;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
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
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{
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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

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
