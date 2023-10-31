/**
 ******************************************************************************
 * @file    mb_port.c
 * @author  Derrick Wang
 * @brief   modebus移植接口
 ******************************************************************************
 * @note
 * 该文件为modbus移植接口的实现，根据不同的MCU平台进行移植
 ******************************************************************************
 */

#include "mb_include.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#ifdef USE_HAL_DRIVER
UART_HandleTypeDef *mb_port_mbTrans_to_uart(uint8_t com_index)
{
	switch (com_index)
	{
	case COM4:
		#if (COM_UART_MAX >= 4)
			return &huart4;
		#else
			return NULL;
		#endif
		break;

	case COM3:
		#if (COM_UART_MAX >= 3)
			return &huart3;
		#else
			return NULL;
		#endif
		break;

	case COM2:
		#if (COM_UART_MAX >= 2)
			return &huart2;
		#else
			return NULL;
		#endif
		break;

	case COM1:
		return &huart1;
		break;

	default:
		return NULL;
		break;
	}
}
#endif

#if (!CubeMX_Setting)
void mb_port_uartInit(uint32_t baud, uint8_t parity)
{
	/*串口部分初始化*/
	huart1.Instance = USART1;
	huart1.Init.BaudRate = baud;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	if (parity == MB_PARITY_ODD)
	{
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_ODD;
	}
	else if (parity == MB_PARITY_EVEN)
	{
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_EVEN;
	}
	else
	{
		huart1.Init.StopBits = UART_STOPBITS_2;
		huart1.Init.Parity = UART_PARITY_NONE;
	}
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}

void mb_port_timerInit(uint32_t baud)
{
	/*定时器部分初始化*/
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 71;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	if (baud > 19200) // 波特率大于19200固定使用1800作为3.5T
	{
		htim3.Init.Period = 1800;
	}
	else // 其他波特率的需要根据计算
	{
		/*	us=1s/(baud/11)*1000000*3.5
		 *			=(11*1000000*3.5)/baud
		 *			=38500000/baud
		 */
		htim3.Init.Period = (uint32_t)38500000 / baud;
	}
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
}
#endif

void mb_port_uartEnable(uint8_t com_index, uint8_t txen, uint8_t rxen)
{
	UART_HandleTypeDef *huart = mb_port_mbTrans_to_uart(com_index);
	if (huart)
	{
		#ifdef USE_HAL_DRIVER
		if (txen)
		{
			__HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
		}
		else
		{
			__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
		}

		if (rxen)
		{
			__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
		}
		else
		{
			__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
		}
		#else
		#endif
	}
}
void mb_port_putchar(USART_TypeDef *Instance, uint8_t ch)
{
	Instance->DR = ch; // 直接操作寄存器比HAL封装的更高效
}

void mb_port_getchar(USART_TypeDef *Instance, uint8_t *ch)
{
	*ch = (uint8_t)(Instance->DR & (uint8_t)0x00FF);
}

#if 0
void mb_port_timerEnable()
{
	__HAL_TIM_DISABLE(&htim3);
	__HAL_TIM_CLEAR_IT(&htim3,TIM_IT_UPDATE);                //清除中断位
	__HAL_TIM_ENABLE_IT(&htim3,TIM_IT_UPDATE);                //使能中断位
	__HAL_TIM_SET_COUNTER(&htim3,0);                           //设置定时器计数为0
	__HAL_TIM_ENABLE(&htim3);                                 //使能定时器
}

void mb_port_timerDisable()
{
	__HAL_TIM_DISABLE(&htim3);
	__HAL_TIM_SET_COUNTER(&htim3,0); 
	__HAL_TIM_DISABLE_IT(&htim3,TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim3,TIM_IT_UPDATE);
}

//串口中断服务函数
void USART1_IRQHandler()
{
	HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	if((__HAL_UART_GET_FLAG(&huart1,UART_FLAG_RXNE)!=RESET))
	{
		__HAL_UART_CLEAR_FLAG(&huart1,UART_FLAG_RXNE);
		mbh_uartRxIsr();
	}
	if((__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TXE)!=RESET))
	{
		__HAL_UART_CLEAR_FLAG(&huart1,UART_FLAG_TXE);
		mbh_uartTxIsr();
	}
}

//定时器中断服务函数
void TIM3_IRQHandler()
{
	__HAL_TIM_CLEAR_IT(&htim3,TIM_IT_UPDATE);
	mbh_timer3T5Isr();
}

#endif