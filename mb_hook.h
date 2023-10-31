/**
  ******************************************************************************
  * @file    mb_hook.c
  * @author  Derrick Wang
  * @brief   modebus回调函数头文件
  ******************************************************************************
  * @note
  * 
  ******************************************************************************
  */

#ifndef __MB_HOOK_H
#define __MB_HOOK_H

/**
 * 	@brief  MODBUS主机模式下接收到从机回复不同功能码的回调处理
 * 	@param	add:从机的地址
 * 	@param 	data:接收到的从机发来的数据指针
 *  @param 	datalen:接收到的从机发来的数据长度
 * 	@return	NONE
 * 	@note	rec01\02\03……等数字代表功能码
 */
void mbh_hook_rec01(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec02(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec03(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec04(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec05(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec06(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec15(uint8_t add, uint8_t *data, uint8_t datalen);
void mbh_hook_rec16(uint8_t add, uint8_t *data, uint8_t datalen);
/**
 * 	@brief  MODBUS主机读写从机超过最大错误次数回调
 *	@param	add:从机的地址
 *	@param  cmd:功能码
 * 	@return	NONE
 * 	@note	
 */
void mbh_hook_timesErr(uint8_t add,uint8_t cmd);

#endif

