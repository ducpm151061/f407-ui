#include "dma.h"
#include "delay.h"

/*****************************************************************************
 * @name       :void DMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
 * @date       :2020-05-08
 * @function   :DMA channel configuration
 * @parameters :DMA_Streamx: DMA stream(DMA1_Stream0~7/DMA2_Stream0~7)
                                chx:DMA channel selection(DMA_Channel_0~DMA_Channel_7)
                                par:Peripheral Address
                                mar:Memory address
                                ndtr:Data transmission volume
 * @retvalue   :None
******************************************************************************/
void DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
{
    DMA_InitTypeDef DMA_InitStructure;
    if ((u32)DMA_Streamx > (u32)DMA2)
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    }
    else
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    }
    DMA_DeInit(DMA_Streamx);
    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } 
    DMA_InitStructure.DMA_Channel = chx;
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = ndtr;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA_Streamx, &DMA_InitStructure);
}

/*****************************************************************************
 * @name       :void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
 * @date       :2020-05-08
 * @function   :Turn on a DMA transfer
 * @parameters :DMA_Streamx: DMA stream(DMA1_Stream0~7/DMA2_Stream0~7)
                                ndtr:Data transmission volume
 * @retvalue   :None
******************************************************************************/
void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, u16 ndtr)
{
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA_Streamx, DISABLE);
    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    }
    DMA_SetCurrDataCounter(DMA_Streamx, ndtr);
    DMA_Cmd(DMA_Streamx, ENABLE);
}
