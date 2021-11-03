#include "spi.h"

/*****************************************************************************
 * @name       :void SPI1_Init(void)
 * @date       :2020-05-08
 * @function   :Initialize the hardware SPI2
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //ʹ��GPIOBʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);  //ʹ��SPI1ʱ��
    // GPIOFB3,4,5��ʼ������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; // PB3~5���ù������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                        //���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                      //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;                  // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                        //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);                              //��ʼ��
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);             // PB3����Ϊ SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);             // PB4����Ϊ SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);             // PB5����Ϊ SPI1

    //����ֻ���SPI�ڳ�ʼ��
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);  //��λSPI1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE); //ֹͣ��λSPI1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // NSS�ź���Ӳ����NSS�ܽţ�����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7; // CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure); //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
    SPI_Cmd(SPI1, ENABLE);    //ʹ��SPI����
    SPI1_ReadWriteByte(0xff); //��������
}

/*****************************************************************************
 * @name       :void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
 * @date       :2020-05-08
 * @function   :set hardware SPI1 speed
 * @parameters :SPI_BaudRatePrescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256
                                TIME:84MHZ
 * @retvalue   :None
******************************************************************************/
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler)); //�ж���Ч��
    SPI1->CR1 &= 0XFFC7;                //λ3-5���㣬�������ò�����
    SPI1->CR1 |= SPI_BaudRatePrescaler; //����SPI1�ٶ�
    SPI_Cmd(SPI1, ENABLE);              //ʹ��SPI1
}

/*****************************************************************************
 * @name       :u8 SPI1_ReadWriteByte(u8 TxData)
 * @date       :2020-05-08
 * @function   :Write a byte of data using hardware SPI
 * @parameters :TxData:Data to be written
 * @retvalue   :Data received by the bus
 ******************************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
    }                               //�ȴ���������
    SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ��byte  ����
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
    }                                 //�ȴ�������һ��byte
    return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����
}
