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
    if ((u32)DMA_Streamx > (u32)DMA2) //�õ���ǰstream������DMA2����DMA1
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2ʱ��ʹ��
    }
    else
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // DMA1ʱ��ʹ��
    }
    DMA_DeInit(DMA_Streamx);
    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } //�ȴ�DMA������
    /* ���� DMA Stream */
    DMA_InitStructure.DMA_Channel = chx;                                    //ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                         // DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                            // DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 //�洢��������ģʽ
    DMA_InitStructure.DMA_BufferSize = ndtr;                                //���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //�洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�������ݳ���:8λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //�洢�����ݳ���:8λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ʹ����ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //�е����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         //�洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //����ͻ�����δ���
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          //��ʼ��DMA Stream
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
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); //ʹ�ܴ���1��DMA����
    DMA_Cmd(DMA_Streamx, DISABLE);                 //�ر�DMA����
    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    }                                          //ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA_Streamx, ndtr); //���ݴ�����
    DMA_Cmd(DMA_Streamx, ENABLE);              //����DMA����
}
