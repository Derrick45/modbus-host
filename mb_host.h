/**
  ******************************************************************************
  * @file    mb_host.h
  * @author  Derrick Wang
  * @brief   modebus主机核心代码头文件
  ******************************************************************************
  * @note
  * 该文件无需修改
  ******************************************************************************
  */



#ifndef __MB_HOST_H
#define __MB_HOST_H

#define MBH_RTU_MIN_SIZE	4
#define MBH_RTU_MAX_SIZE	255	//最大不超过255
#define MBH_ERR_MAX_TIMES	3
#define MBH_REC_TIMEOUT		100  //单位3.5T

typedef enum
{
	MBH_STATE_IDLE=0X00,
	MBH_STATE_TX,
	MBH_STATE_TX_END,
	MBH_STATE_RX,
	MBH_STATE_RX_CHECK,
	MBH_STATE_EXEC,
	MBH_STATE_REC_ERR,		//接收错误状态
	MBH_STATE_TIMES_ERR,	//传输
	
}mb_host_state;

/**
 * 	@brief  MODBUS主机初始化
 * 	@param	baud:串口波特率
 * 	@param 	parity:奇偶校验位设置	
 * 	@return	NONE
 * 	@note	使用modbus之前先调用该函数进行初始化
 */
void mbh_init(uint32_t baud,uint8_t parity);
/**
 * 	@brief  MODBUS主机给从机发送一条命令
 * 	@param	add:从机地址
 * 	@param 	cmd:功能码
 *	@param	data:要发送的数据
 *	@param	len:发送的数据长度
 * 	@return	-1:发送失败	0:发送成功
 * 	@note	该函数为非阻塞式，调用后立即返回
 */
int8_t mbh_send(uint8_t add,uint8_t cmd,uint8_t *data,uint8_t data_len);
/**
 * 	@brief  获取MODBUS主机运行状态
 * 	@return	mb_host_state中状态
 * 	@note	
 */
uint8_t mbh_getState(void);
/**
 * 	@brief  MODBUS状态轮训
 * 	@return	none
 * 	@note	该函数必须不断轮询，用来底层核心状态进行切换
 *			可在操作系统任务中运行，但应该尽可能短的延时间隔
 */
void mbh_poll(void);

/**
* 	@brief  modbus主机定时器中断处理
 * 	@return	none
 * 	@note	放在mcu的中断服务中调用
 *			
 */
void mbh_timer3T5Isr(void);
/**
 * 	@brief  modbus主机串口接收中断处理
 * 	@return	none
 * 	@note	放在mcu的rx中断中调用
 *			
 */
void mbh_uartRxIsr(void);
/**
 * 	@brief  modbus主机串口接收中断处理
 * 	@return	none
 * 	@note	放在mcu的tx中断中调用
 *			
 */
void mbh_uartTxIsr(void);


#endif

