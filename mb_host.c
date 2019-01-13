/**
  ******************************************************************************
  * @file    mb_host.c
  * @author  Derrick Wang
  * @brief   modebus主机实现代码
  ******************************************************************************
  * @note
  * 该文件无需修改
  ******************************************************************************
  */

#include "mb_include.h"
#include "string.h"

struct
{
	uint8_t state;						//modbus host状态
	uint8_t errTimes;  					//失败次数计数
	uint8_t txLen;     					//需要发送的帧长度
	uint8_t txCounter;					//已发送bytes计数
	uint8_t txBuf[MBH_RTU_MAX_SIZE];	//发送缓冲区
	uint8_t rxCounter;					//接收计数
	uint8_t rxBuf[MBH_RTU_MAX_SIZE];	//接收缓冲区
	uint8_t rxTimeOut;					//接收时的超时计数
	
}mbHost;


//modbus初始化
void mbh_init(uint32_t baud,uint8_t parity)
{
	mb_port_uartInit(baud,parity);
	mb_port_timerInit(baud);
}
uint8_t mbh_getState()
{
	return mbHost.state;
}

//发送一帧命令
int8_t mbh_send(uint8_t add,uint8_t cmd,uint8_t *data,uint8_t data_len)
{
	uint16_t crc;
	if(mbHost.state!=MBH_STATE_IDLE)return -1; //busy state

	mbHost.txCounter=0;
	mbHost.rxCounter=0;	
	mbHost.txBuf[0]=add;
	mbHost.txBuf[1]=cmd;
	memcpy((mbHost.txBuf+2),data,data_len);
	mbHost.txLen=data_len+2; //data(n)+add(1)+cmd(1)
	crc=mb_crc16(mbHost.txBuf,mbHost.txLen);
	mbHost.txBuf[mbHost.txLen++]=(uint8_t)(crc&0xff);
	mbHost.txBuf[mbHost.txLen++]=(uint8_t)(crc>>8);
	
	mbHost.state=MBH_STATE_TX;
	mb_port_uartEnable(1,0);  //enable tx,disable rx
	/*当打开TXE中断以后，立马就会触发一次，所以这里不用先发送一个byte*/
	//mb_port_putchar(mbHost.txBuf[mbHost.txCounter++]); //send first char,then enter tx isr
	return 0;
}
//接收正确,进行解析处理
void mbh_exec(uint8_t *pframe,uint8_t len)
{
	uint8_t datalen=len-2;
	switch(pframe[1])//cmd
	{
		case 1:
			mbh_hook_rec01(pframe[0],(pframe+2),datalen);
			break;
		case 2:
			mbh_hook_rec02(pframe[0],(pframe+2),datalen);
			break;
		case 3:
			mbh_hook_rec03(pframe[0],(pframe+2),datalen);
			break;
		case 4:
			mbh_hook_rec04(pframe[0],(pframe+2),datalen);
			break;
		case 5:
			mbh_hook_rec05(pframe[0],(pframe+2),datalen);
			break;
		case 6:
			mbh_hook_rec06(pframe[0],(pframe+2),datalen);
			break;
		case 15:
			mbh_hook_rec15(pframe[0],(pframe+2),datalen);
			break;
		case 16:
			mbh_hook_rec16(pframe[0],(pframe+2),datalen);
			break;
	}
}
void mbh_poll()
{
	switch(mbHost.state)
	{
		/*接收完一帧数据,开始进行校验*/
		case MBH_STATE_RX_CHECK:  //接收完成，对一帧数据进行检查
			if((mbHost.rxCounter>=MBH_RTU_MIN_SIZE)&&(mb_crc16(mbHost.rxBuf,mbHost.rxCounter)==0)) 	//接收的一帧数据正确
			{
				if((mbHost.txBuf[0]==mbHost.rxBuf[0])&&(mbHost.txBuf[1]==mbHost.rxBuf[1]))			//发送帧数据和接收到的帧数据地址和功能码一样
				{
					mbHost.state=MBH_STATE_EXEC;
				}
				else mbHost.state=MBH_STATE_REC_ERR;
				
			}
			else mbHost.state=MBH_STATE_REC_ERR;
			break;
		/*接收一帧数据出错*/	
		case MBH_STATE_REC_ERR:
			mbHost.errTimes++;			
			if(mbHost.errTimes>=MBH_ERR_MAX_TIMES)  
			{
				mbHost.state=MBH_STATE_TIMES_ERR;
			}
			else  //重新再启动一次传输
			{
				mbHost.txCounter=0;
				mbHost.rxCounter=0;
				mbHost.state=MBH_STATE_TX;
				mb_port_uartEnable(1,0);  //enable tx,disable rx
			}
			break;
		/*超过最大错误传输次数*/
		case MBH_STATE_TIMES_ERR:
			mbh_hook_timesErr(mbHost.txBuf[0],mbHost.txBuf[1]);
			mbHost.txCounter=0;
			mbHost.rxCounter=0;
			break;
		/*确定接收正确执行回调*/
		case MBH_STATE_EXEC:      //主机发送接收完成，执行回调
			mbh_exec(mbHost.rxBuf,mbHost.rxCounter);
			mbHost.state=MBH_STATE_IDLE;
			break;
		
	}
}


void mbh_timer3T5Isr()
{
	switch(mbHost.state)
	{
		/*发送完但没有接收到数据*/
		case MBH_STATE_TX_END:
			mbHost.rxTimeOut++;
			if(mbHost.rxTimeOut>=MBH_REC_TIMEOUT) //接收超时
			{
				mbHost.rxTimeOut=0;
				mbHost.state=MBH_STATE_REC_ERR;
				mb_port_timerDisable();		//关闭定时器
				mb_port_uartEnable(0,0); 	//串口tx、rx都关闭
			}
			break;
		case MBH_STATE_RX:     	//3.5T到,接收一帧完成			
			mbHost.state=MBH_STATE_RX_CHECK;
			mb_port_timerDisable();		//关闭定时器
			mb_port_uartEnable(0,0); 	//串口tx、rx都关闭
			break;
	}
	
}
void mbh_uartRxIsr()
{
	uint8_t ch;
	mb_port_getchar(&ch);
	switch(mbHost.state)
	{
		case MBH_STATE_TX_END:
			mbHost.rxCounter=0;
			mbHost.rxBuf[mbHost.rxCounter++]=ch;
			mbHost.state=MBH_STATE_RX;
			mb_port_timerEnable();
			break;
		case MBH_STATE_RX:
			if(mbHost.rxCounter<MBH_RTU_MAX_SIZE)
			{
				mbHost.rxBuf[mbHost.rxCounter++]=ch;
			}
			mb_port_timerEnable();
			break;
		default:
			mb_port_timerEnable();
			break;		
	}
}
void mbh_uartTxIsr()
{
	switch (mbHost.state)
	{
		case MBH_STATE_TX:
			if(mbHost.txCounter==mbHost.txLen) //全部发送完
			{
				mbHost.state=MBH_STATE_TX_END;
				mb_port_uartEnable(0,1);  //disable tx,enable rx
				mbHost.rxTimeOut=0;		  //清除接收超时计数
				mb_port_timerEnable();    //open timer
			}
			else
			{
				mb_port_putchar(mbHost.txBuf[mbHost.txCounter++]);
			}
			break;
		case MBH_STATE_TX_END:
			mb_port_uartEnable(0,1);  	  //disable tx,enable rx
			break;
	}	
}



