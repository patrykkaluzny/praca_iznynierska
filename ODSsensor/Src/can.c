/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

#include "gpio.h"

/* USER CODE BEGIN 0 */

#include "CANflags.h"
#include "init_functions.h"
#include "general_functions.h"
#include "stm32l432xx.h"
#include "structures.h"
#include "errorFlags.h"
//can variables
CAN_FilterTypeDef fcan1;
CAN_RxHeaderTypeDef rFrame;
volatile uint8_t rFrameData[8];
extern volatile typ_CANTxBuffer CANTxBuffer;

//boardData structure
extern volatile typ_DataToSend				boardData;
// timers variables
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim15;
//boolean
uint8_t Is_CAN_INIT_ADDRESS_Recive=0; //variable used in process of id granting
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
  
    /**CAN1 GPIO Configuration    
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN1 GPIO Configuration    
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void CAN_SetFilter(void)
{

	  fcan1.FilterBank=0;
	  fcan1.FilterMode=CAN_FILTERMODE_IDMASK;
	  fcan1.FilterScale=CAN_FILTERSCALE_16BIT;
	  fcan1.FilterIdHigh=0x0000;
	  fcan1.FilterIdLow=0x0000;
	  fcan1.FilterMaskIdHigh=0x0000;
	  fcan1.FilterMaskIdLow=0x0000;
	  fcan1.FilterFIFOAssignment=0;
	  fcan1.FilterActivation=ENABLE;
	  HAL_CAN_ConfigFilter(&hcan1,&fcan1);

}

uint8_t CAN_Transsmit(CAN_TxHeaderTypeDef *FrameToSend,uint8_t *DataToSend)
{
	uint8_t transmissionStatus;
	transmissionStatus= HAL_CAN_AddTxMessage(&hcan1,FrameToSend,DataToSend,(void*)(long)CAN_TX_MAILBOX0);
	if(transmissionStatus==HAL_ERROR)
	{
		transmissionStatus= HAL_CAN_AddTxMessage(&hcan1,FrameToSend,DataToSend,(void*)(long)CAN_TX_MAILBOX1);
	}
	if(transmissionStatus==HAL_ERROR)
	{
		transmissionStatus= HAL_CAN_AddTxMessage(&hcan1,FrameToSend,DataToSend,(void*)(long)CAN_TX_MAILBOX2);
	}
	if(transmissionStatus==HAL_ERROR)
	{
		return HAL_ERROR;
	}
	else{return 0;}
}
void CAN_Decode(CAN_RxHeaderTypeDef *reciveFrame, uint8_t *reciveDataData)
 {
	switch (reciveFrame->StdId) {
	case _CAN_INIT_ADDRESS:
		CAN_DecodeInitAddress(reciveFrame, reciveDataData);
		break;
	case _CAN_INIT_ADDRESS2:
		CAN_DecodeInitAddress2(reciveFrame, reciveDataData);
		break;
	case _CAN_INIT_TIME:
		CAN_DecodeInitTime(reciveFrame, reciveDataData);
		break;
	default:
		break;
	}
}
void CAN_FramesInit(void)
{
	rFrame.StdId=0x01;
	rFrame.ExtId=0x01;
	rFrame.IDE=CAN_ID_STD;;
	rFrame.RTR=CAN_RTR_DATA;
	rFrame.FilterMatchIndex=0x00;
	rFrame.DLC=8;
	for (int k = 0; k < 8; k++) {
		rFrameData[k] = 0;
	}
}
void CAN_SendFrameWithBuffer(uint32_t CANStdId, uint32_t *CANData1, uint32_t *CANData2)
{

	uint8_t transmissionStatus;
	// temporary frame variable
	CAN_TxHeaderTypeDef tempFrame;
	uint8_t tempFrameData[8];
	// filling frame
	tempFrame.StdId = CANStdId;
	tempFrame.ExtId = 0x00;
	tempFrame.IDE = CAN_ID_STD;
	tempFrame.RTR = CAN_RTR_DATA;
	tempFrame.DLC = 8;
	for (int k = 0; k < 8; k++) {
		tempFrameData[k] = 0;
	}
	Double_U32_to_U8_Array(tempFrameData,CANData1,CANData2); //copy data
	if (TIM16->CR1 & TIM_CR1_CEN) {
		CAN_Add2Buffer(&tempFrame, CANData1, CANData2);
		if (CANTxBuffer.sFrameBufferOVF) {
			SetBits(boardData.errorFlag,ODSSENSOR_ERROR_CAN_BUFF_OVF);
		}
	}
	else {
		transmissionStatus = CAN_Transsmit(&tempFrame, tempFrameData);
		if (transmissionStatus == HAL_ERROR) {
			CAN_Add2Buffer(&tempFrame, CANData1, CANData2);
			{
				if (CANTxBuffer.sFrameBufferOVF) {
					SetBits(boardData.errorFlag,ODSSENSOR_ERROR_CAN_BUFF_OVF);
				}
			}
		}
	}
}
uint8_t CAN_Add2Buffer(CAN_TxHeaderTypeDef *CANPackage, uint32_t *CANData1,uint32_t *CANData2)
{
	//check if buffer is overflow
	if (!CAN_CheckOverflow()) {
		// add frame to buffer
		CANTxBuffer.sFrameBuffer[CANTxBuffer.sFrameBufferHead] = *CANPackage;
		CANTxBuffer.sCANData1[CANTxBuffer.sFrameBufferHead]= *CANData1;
		CANTxBuffer.sCANData2[CANTxBuffer.sFrameBufferHead]= *CANData2;
		// counter increment
		CANTxBuffer.sFrameBufferHead++;
		if (CANTxBuffer.sFrameBufferHead >= _CAN_TX_BUFFER_SIZE)
		{
			CANTxBuffer.sFrameBufferHead = 0;
		}
		// enable timer16
		HAL_TIM_Base_Start(&htim16);
		return 1;
	} else {
		SetBits(boardData.errorFlag,ODSSENSOR_ERROR_CAN_BUFF_OVF);
		CANTxBuffer.sFrameBufferOVF = 1;
		return 0;
	}
}
uint8_t CAN_ResendFrame(volatile CAN_TxHeaderTypeDef *CANPackage, volatile uint32_t *CANData1,volatile uint32_t *CANData2)
{
	uint8_t tempFrameData[8];
	uint8_t transmissionStatus;
	Double_U32_to_U8_Array(tempFrameData,CANData1,CANData2);
	transmissionStatus=CAN_Transsmit(CANPackage,tempFrameData);
	return transmissionStatus;

}
uint8_t CAN_CheckOverflow(void)
{
	// | | |T|x|x|x|H| |
	if(CANTxBuffer.sFrameBufferTail != 0)
	{
		// |x|H|T|x|x|x|x|x|
		if(CANTxBuffer.sFrameBufferHead == CANTxBuffer.sFrameBufferTail-1)
		{
			return 1;
		// |x|H| | | |T|x|x|
		}else
		{
			return 0;
		}
	// |T|x|x|x|H| | | |
	}else // == 0
	{
		// |T|x|x|x|x|x|x|H|
		if(CANTxBuffer.sFrameBufferHead == _CAN_TX_BUFFER_SIZE-1)
		{
			return 1;
		// |T|x|x|x|x|H| | |
		}else
		{
			return 0;
		}
	}
}

void CAN_DecodeInitAddress(CAN_RxHeaderTypeDef *RxMessage, uint8_t *RxData)
{
	if((U8_Array_to_U32(&RxData[0])==STM32_UUID[0])&&(U8_Array_to_U32(&RxData[4]) == STM32_UUID[1]))
	{
		Is_CAN_INIT_ADDRESS_Recive=1;
	}

}
void CAN_DecodeInitAddress2(CAN_RxHeaderTypeDef *RxMessage, uint8_t *RxData)
{
	if (Is_CAN_INIT_ADDRESS_Recive) {
		boardData.ID = (uint8_t) (U8_Array_to_U32(&RxData[0]));
		Is_CAN_INIT_ADDRESS_Recive=0;
		ResetBits(boardData.errorFlag,ODSSENSOR_ERROR_ID);
		HAL_TIM_Base_Start_IT(&htim15); // timer 15 shows id number of sensorboard
		HAL_TIM_Base_Start_IT(&htim2);  // interrupt of tim2 sends data from sensorboard
	}

}
void CAN_DecodeInitTime(CAN_RxHeaderTypeDef *RxMessage, uint8_t *RxData)
{
	uint32_t serialID_1 = U8_Array_to_U32(&RxData[0]);
	uint32_t serialID_2 = U8_Array_to_U32(&RxData[4]);
	if((serialID_1==STM32_UUID[0])&&(serialID_2 == STM32_UUID[1]))
	{
		CAN_SendFrameWithBuffer(_CAN_INIT_TIME,&serialID_1,&serialID_2);
	}
}
void CAN_SendDistancAndErrorsData()
{
	CAN_SendFrameWithBuffer(_CAN_DATA_ERROR,&boardData.ID,&boardData.errorFlag);
	CAN_SendFrameWithBuffer(_CAN_DATA_DISTANCE,&boardData.ID,&boardData.distanceValue);
}
void CAN_SendTempAndLuxData()
{
	uint32_t *address;
	address = (uint32_t *)&boardData.luxValue;
	CAN_SendFrameWithBuffer(_CAN_DATA_LUX,&boardData.ID,address);
	address = (uint32_t *)&boardData.temperatureValue;
	CAN_SendFrameWithBuffer(_CAN_DATA_TEMP,&boardData.ID,address);
}
/* USER CODE END 1 */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
