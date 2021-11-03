#include "sdio_sdcard.h"
#include "string.h"
#include "sys.h"
#include "usart.h"

SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;

SD_Error CmdError(void);
SD_Error CmdResp7Error(void);
SD_Error CmdResp1Error(u8 cmd);
SD_Error CmdResp3Error(void);
SD_Error CmdResp2Error(void);
SD_Error CmdResp6Error(u8 cmd, u16 *prca);
SD_Error SDEnWideBus(u8 enx);
SD_Error IsCardProgramming(u8 *pstatus);
SD_Error FindSCR(u16 rca, u32 *pscr);
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes);

static u8 CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
static u32 CSD_Tab[4], CID_Tab[4], RCA = 0;
static u8 DeviceMode = SD_DMA_MODE;
static u8 StopCondition = 0;
volatile SD_Error TransferError = SD_OK;
volatile u8 TransferEnd = 0;
SD_CardInfo SDCardInfo;

#pragma *pack *(4)
u8 SDIO_DATA_BUFFER[512];

void SDIO_Register_Deinit()
{
    SDIO->POWER = 0x00000000;
    SDIO->CLKCR = 0x00000000;
    SDIO->ARG = 0x00000000;
    SDIO->CMD = 0x00000000;
    SDIO->DTIMER = 0x00000000;
    SDIO->DLEN = 0x00000000;
    SDIO->DCTRL = 0x00000000;
    SDIO->ICR = 0x00C007FF;
    SDIO->MASK = 0x00000000;
}

SD_Error SD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    SD_Error errorstatus = SD_OK;
    u8 clkdiv = 0;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); //ʹ��GPIOGʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;             // PG9�������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;         //�������
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //����
    GPIO_Init(GPIOG, &GPIO_InitStructure);                //��ʼ��
    GPIO_SetBits(GPIOG, GPIO_Pin_9);                      //����

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2,
                           ENABLE);                      //ʹ��GPIOC,GPIOD DMA2ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE); // SDIOʱ��ʹ��
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE); // SDIO��λ

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12; // PC8,9,10,11,12���ù������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                           //���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                      // 100M
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
    GPIO_Init(GPIOC, &GPIO_InitStructure);       // PC8,9,10,11,12���ù������

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);                  // PD2���ù������
                                                            //���Ÿ���ӳ������
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO); // PC8,AF12
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE); // SDIO������λ

    // SDIO����Ĵ�������ΪĬ��ֵ
    SDIO_Register_Deinit();

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure); //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

    errorstatus = SD_PowerON(); // SD���ϵ�
    if (errorstatus == SD_OK)
        errorstatus = SD_InitializeCards(); //��ʼ��SD��
    if (errorstatus == SD_OK)
        errorstatus = SD_GetCardInfo(&SDCardInfo); //��ȡ����Ϣ
    if (errorstatus == SD_OK)
        errorstatus = SD_SelectDeselect((u32)(SDCardInfo.RCA << 16)); //ѡ��SD��
    if (errorstatus == SD_OK)
        errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b); // 4λ����,�����MMC��,������4λģʽ
    if ((errorstatus == SD_OK) || (SDIO_MULTIMEDIA_CARD == CardType))
    {
        if (SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 ||
            SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)
        {
            clkdiv = SDIO_TRANSFER_CLK_DIV + 2; // V1.1/V2.0�����������48/4=12Mhz
        }
        else
            clkdiv = SDIO_TRANSFER_CLK_DIV; // SDHC�����������������48/2=24Mhz
        SDIO_Clock_Set(clkdiv); //����ʱ��Ƶ��,SDIOʱ�Ӽ��㹫ʽ:SDIO_CKʱ��=SDIOCLK/[clkdiv+2];����,SDIOCLK�̶�Ϊ48Mhz
        // errorstatus=SD_SetDeviceMode(SD_DMA_MODE);	//����ΪDMAģʽ
        errorstatus = SD_SetDeviceMode(SD_POLLING_MODE); //����Ϊ��ѯģʽ
    }
    return errorstatus;
}
// SDIOʱ�ӳ�ʼ������
// clkdiv:ʱ�ӷ�Ƶϵ��
// CKʱ��=SDIOCLK/[clkdiv+2];(SDIOCLKʱ�ӹ̶�Ϊ48Mhz)
void SDIO_Clock_Set(u8 clkdiv)
{
    u32 tmpreg = SDIO->CLKCR;
    tmpreg &= 0XFFFFFF00;
    tmpreg |= clkdiv;
    SDIO->CLKCR = tmpreg;
}

//���ϵ�
//��ѯ����SDIO�ӿ��ϵĿ��豸,����ѯ���ѹ������ʱ��
//����ֵ:�������;(0,�޴���)
SD_Error SD_PowerON(void)
{
    u8 i = 0;
    SD_Error errorstatus = SD_OK;
    u32 response = 0, count = 0, validvoltage = 0;
    u32 SDType = SD_STD_CAPACITY;

    /*��ʼ��ʱ��ʱ�Ӳ��ܴ���400KHz*/
    SDIO_InitStructure.SDIO_ClockDiv =
        SDIO_INIT_CLK_DIV; /* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
    SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
    SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable; //��ʹ��bypassģʽ��ֱ����HCLK���з�Ƶ�õ�SDIO_CK
    SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable; // ����ʱ���ر�ʱ�ӵ�Դ
    SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;                    // 1λ������
    SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable; //Ӳ����
    SDIO_Init(&SDIO_InitStructure);

    SDIO_SetPowerState(SDIO_PowerState_ON); //�ϵ�״̬,������ʱ��
    SDIO->CLKCR |= 1 << 8;                  // SDIOCKʹ��

    for (i = 0; i < 74; i++)
    {

        SDIO_CmdInitStructure.SDIO_Argument = 0x0; //����CMD0����IDLE STAGEģʽ����.
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE; // cmd0
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;     //����Ӧ
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable; //��CPSM�ڿ�ʼ��������֮ǰ�ȴ����ݴ��������
        SDIO_SendCommand(&SDIO_CmdInitStructure); //д���������Ĵ���

        errorstatus = CmdError();

        if (errorstatus == SD_OK)
            break;
    }
    if (errorstatus)
        return errorstatus; //���ش���״̬

    SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN; //����CMD8,����Ӧ,���SD���ӿ�����
    SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;   // cmd8
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r7
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;            //�رյȴ��ж�
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp7Error(); //�ȴ�R7��Ӧ

    if (errorstatus == SD_OK) // R7��Ӧ����
    {
        CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; // SD 2.0��
        SDType = SD_HIGH_CAPACITY;                 //��������
    }

    SDIO_CmdInitStructure.SDIO_Argument = 0x00; //����CMD55,����Ӧ
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD55,����Ӧ

    errorstatus = CmdResp1Error(SD_CMD_APP_CMD); //�ȴ�R1��Ӧ

    if (errorstatus == SD_OK) // SD2.0/SD 1.1,����ΪMMC��
    {
        // SD��,����ACMD41 SD_APP_OP_COND,����Ϊ:0x80100000
        while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
        {
            SDIO_CmdInitStructure.SDIO_Argument = 0x00;           //����CMD55,����Ӧ
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD; // CMD55
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD55,����Ӧ

            errorstatus = CmdResp1Error(SD_CMD_APP_CMD); //�ȴ�R1��Ӧ

            if (errorstatus != SD_OK)
                return errorstatus; //��Ӧ����

            // acmd41�����������֧�ֵĵ�ѹ��Χ��HCSλ��ɣ�HCSλ��һ�����ֿ���SDSc����sdhc
            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType; //����ACMD41,����Ӧ
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r3
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp3Error(); //�ȴ�R3��Ӧ

            if (errorstatus != SD_OK)
                return errorstatus; //��Ӧ����
            response = SDIO->RESP1;
            ;                                                 //�õ���Ӧ
            validvoltage = (((response >> 31) == 1) ? 1 : 0); //�ж�SD���ϵ��Ƿ����
            count++;
        }
        if (count >= SD_MAX_VOLT_TRIAL)
        {
            errorstatus = SD_INVALID_VOLTRANGE;
            return errorstatus;
        }
        if (response &= SD_HIGH_CAPACITY)
        {
            CardType = SDIO_HIGH_CAPACITY_SD_CARD;
        }
    }
    else // MMC��
    {
        // MMC��,����CMD1 SDIO_SEND_OP_COND,����Ϊ:0x80FF8000
        while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
        {
            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC; //����CMD1,����Ӧ
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r3
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp3Error(); //�ȴ�R3��Ӧ

            if (errorstatus != SD_OK)
                return errorstatus; //��Ӧ����
            response = SDIO->RESP1;
            ; //�õ���Ӧ
            validvoltage = (((response >> 31) == 1) ? 1 : 0);
            count++;
        }
        if (count >= SD_MAX_VOLT_TRIAL)
        {
            errorstatus = SD_INVALID_VOLTRANGE;
            return errorstatus;
        }
        CardType = SDIO_MULTIMEDIA_CARD;
    }
    return (errorstatus);
}
// SD�� Power OFF
//����ֵ:�������;(0,�޴���)
SD_Error SD_PowerOFF(void)
{

    SDIO_SetPowerState(SDIO_PowerState_OFF); // SDIO��Դ�ر�,ʱ��ֹͣ

    return SD_OK;
}
//��ʼ�����еĿ�,���ÿ��������״̬
//����ֵ:�������
SD_Error SD_InitializeCards(void)
{
    SD_Error errorstatus = SD_OK;
    u16 rca = 0x01;

    if (SDIO_GetPowerState() == SDIO_PowerState_OFF) //����Դ״̬,ȷ��Ϊ�ϵ�״̬
    {
        errorstatus = SD_REQUEST_NOT_APPLICABLE;
        return (errorstatus);
    }

    if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) //��SECURE_DIGITAL_IO_CARD
    {
        SDIO_CmdInitStructure.SDIO_Argument = 0x0; //����CMD2,ȡ��CID,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD2,ȡ��CID,����Ӧ

        errorstatus = CmdResp2Error(); //�ȴ�R2��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����

        CID_Tab[0] = SDIO->RESP1;
        CID_Tab[1] = SDIO->RESP2;
        CID_Tab[2] = SDIO->RESP3;
        CID_Tab[3] = SDIO->RESP4;
    }
    if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
        (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType)) //�жϿ�����
    {
        SDIO_CmdInitStructure.SDIO_Argument = 0x00;                //����CMD3,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR; // cmd3
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r6
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD3,����Ӧ

        errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca); //�ȴ�R6��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����
    }
    if (SDIO_MULTIMEDIA_CARD == CardType)
    {

        SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca << 16);    //����CMD3,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR; // cmd3
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r6
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD3,����Ӧ

        errorstatus = CmdResp2Error(); //�ȴ�R2��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����
    }
    if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) //��SECURE_DIGITAL_IO_CARD
    {
        RCA = rca;

        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16); //����CMD9+��RCA,ȡ��CSD,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp2Error(); //�ȴ�R2��Ӧ
        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����

        CSD_Tab[0] = SDIO->RESP1;
        CSD_Tab[1] = SDIO->RESP2;
        CSD_Tab[2] = SDIO->RESP3;
        CSD_Tab[3] = SDIO->RESP4;
    }
    return SD_OK; //����ʼ���ɹ�
}
//�õ�����Ϣ
// cardinfo:����Ϣ�洢��
//����ֵ:����״̬
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
    SD_Error errorstatus = SD_OK;
    u8 tmp = 0;
    cardinfo->CardType = (u8)CardType; //������
    cardinfo->RCA = (u16)RCA;          //��RCAֵ
    tmp = (u8)((CSD_Tab[0] & 0xFF000000) >> 24);
    cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6; // CSD�ṹ
    cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2; // 2.0Э�黹û�����ⲿ��(Ϊ����),Ӧ���Ǻ���Э�鶨���
    cardinfo->SD_csd.Reserved1 = tmp & 0x03;               // 2������λ
    tmp = (u8)((CSD_Tab[0] & 0x00FF0000) >> 16);           //��1���ֽ�
    cardinfo->SD_csd.TAAC = tmp;                           //���ݶ�ʱ��1
    tmp = (u8)((CSD_Tab[0] & 0x0000FF00) >> 8);            //��2���ֽ�
    cardinfo->SD_csd.NSAC = tmp;                           //���ݶ�ʱ��2
    tmp = (u8)(CSD_Tab[0] & 0x000000FF);                   //��3���ֽ�
    cardinfo->SD_csd.MaxBusClkFrec = tmp;                  //�����ٶ�
    tmp = (u8)((CSD_Tab[1] & 0xFF000000) >> 24);           //��4���ֽ�
    cardinfo->SD_csd.CardComdClasses = tmp << 4;           //��ָ�������λ
    tmp = (u8)((CSD_Tab[1] & 0x00FF0000) >> 16);           //��5���ֽ�
    cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4; //��ָ�������λ
    cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;              //����ȡ���ݳ���
    tmp = (u8)((CSD_Tab[1] & 0x0000FF00) >> 8);            //��6���ֽ�
    cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;    //�����ֿ��
    cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;  //д���λ
    cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;  //�����λ
    cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.Reserved2 = 0; //����
    if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) ||
        (SDIO_MULTIMEDIA_CARD == CardType)) //��׼1.1/2.0��/MMC��
    {
        cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10; // C_SIZE(12λ)
        tmp = (u8)(CSD_Tab[1] & 0x000000FF);              //��7���ֽ�
        cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
        tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24); //��8���ֽ�
        cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
        cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
        cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
        tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16); //��9���ֽ�
        cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
        cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
        cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1; // C_SIZE_MULT
        tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);         //��10���ֽ�
        cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
        cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1); //���㿨����
        cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
        cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen); //���С
        cardinfo->CardCapacity *= cardinfo->CardBlockSize;
    }
    else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //��������
    {
        tmp = (u8)(CSD_Tab[1] & 0x000000FF);              //��7���ֽ�
        cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16; // C_SIZE
        tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);      //��8���ֽ�
        cardinfo->SD_csd.DeviceSize |= (tmp << 8);
        tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16); //��9���ֽ�
        cardinfo->SD_csd.DeviceSize |= (tmp);
        tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);                                         //��10���ֽ�
        cardinfo->CardCapacity = (long long)(cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024; //���㿨����
        cardinfo->CardBlockSize = 512; //���С�̶�Ϊ512�ֽ�
    }
    cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;
    tmp = (u8)(CSD_Tab[2] & 0x000000FF); //��11���ֽ�
    cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);
    tmp = (u8)((CSD_Tab[3] & 0xFF000000) >> 24); //��12���ֽ�
    cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
    cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;
    tmp = (u8)((CSD_Tab[3] & 0x00FF0000) >> 16); //��13���ֽ�
    cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.Reserved3 = 0;
    cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);
    tmp = (u8)((CSD_Tab[3] & 0x0000FF00) >> 8); //��14���ֽ�
    cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
    cardinfo->SD_csd.ECC = (tmp & 0x03);
    tmp = (u8)(CSD_Tab[3] & 0x000000FF); //��15���ֽ�
    cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_csd.Reserved4 = 1;
    tmp = (u8)((CID_Tab[0] & 0xFF000000) >> 24); //��0���ֽ�
    cardinfo->SD_cid.ManufacturerID = tmp;
    tmp = (u8)((CID_Tab[0] & 0x00FF0000) >> 16); //��1���ֽ�
    cardinfo->SD_cid.OEM_AppliID = tmp << 8;
    tmp = (u8)((CID_Tab[0] & 0x000000FF00) >> 8); //��2���ֽ�
    cardinfo->SD_cid.OEM_AppliID |= tmp;
    tmp = (u8)(CID_Tab[0] & 0x000000FF); //��3���ֽ�
    cardinfo->SD_cid.ProdName1 = tmp << 24;
    tmp = (u8)((CID_Tab[1] & 0xFF000000) >> 24); //��4���ֽ�
    cardinfo->SD_cid.ProdName1 |= tmp << 16;
    tmp = (u8)((CID_Tab[1] & 0x00FF0000) >> 16); //��5���ֽ�
    cardinfo->SD_cid.ProdName1 |= tmp << 8;
    tmp = (u8)((CID_Tab[1] & 0x0000FF00) >> 8); //��6���ֽ�
    cardinfo->SD_cid.ProdName1 |= tmp;
    tmp = (u8)(CID_Tab[1] & 0x000000FF); //��7���ֽ�
    cardinfo->SD_cid.ProdName2 = tmp;
    tmp = (u8)((CID_Tab[2] & 0xFF000000) >> 24); //��8���ֽ�
    cardinfo->SD_cid.ProdRev = tmp;
    tmp = (u8)((CID_Tab[2] & 0x00FF0000) >> 16); //��9���ֽ�
    cardinfo->SD_cid.ProdSN = tmp << 24;
    tmp = (u8)((CID_Tab[2] & 0x0000FF00) >> 8); //��10���ֽ�
    cardinfo->SD_cid.ProdSN |= tmp << 16;
    tmp = (u8)(CID_Tab[2] & 0x000000FF); //��11���ֽ�
    cardinfo->SD_cid.ProdSN |= tmp << 8;
    tmp = (u8)((CID_Tab[3] & 0xFF000000) >> 24); //��12���ֽ�
    cardinfo->SD_cid.ProdSN |= tmp;
    tmp = (u8)((CID_Tab[3] & 0x00FF0000) >> 16); //��13���ֽ�
    cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
    cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;
    tmp = (u8)((CID_Tab[3] & 0x0000FF00) >> 8); //��14���ֽ�
    cardinfo->SD_cid.ManufactDate |= tmp;
    tmp = (u8)(CID_Tab[3] & 0x000000FF); //��15���ֽ�
    cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_cid.Reserved2 = 1;
    return errorstatus;
}
SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
    SD_Error errorstatus = SD_OK;
    if (SDIO_MULTIMEDIA_CARD == CardType)
    {
        errorstatus = SD_UNSUPPORTED_FEATURE;
        return (errorstatus);
    }
    else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
             (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
    {
        if (SDIO_BusWide_8b == WideMode) // 2.0 sd��֧��8bits
        {
            errorstatus = SD_UNSUPPORTED_FEATURE;
            return (errorstatus);
        }
        else
        {
            errorstatus = SDEnWideBus(WideMode);
            if (SD_OK == errorstatus)
            {
                SDIO->CLKCR &= ~(3 << 11); //���֮ǰ��λ������
                SDIO->CLKCR |= WideMode;   // 1λ/4λ���߿���
                SDIO->CLKCR |= 0 << 14;    //������Ӳ��������
            }
        }
    }
    return errorstatus;
}
SD_Error SD_SetDeviceMode(u32 Mode)
{
    SD_Error errorstatus = SD_OK;
    if ((Mode == SD_DMA_MODE) || (Mode == SD_POLLING_MODE))
        DeviceMode = Mode;
    else
        errorstatus = SD_INVALID_PARAMETER;
    return errorstatus;
}
SD_Error SD_SelectDeselect(u32 addr)
{

    SDIO_CmdInitStructure.SDIO_Argument = addr; //����CMD7,ѡ��,����Ӧ
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD7,ѡ��,����Ӧ

    return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
}
// SD����ȡһ����
// buf:�����ݻ�����(����4�ֽڶ���!!)
// addr:��ȡ��ַ
// blksize:���С
SD_Error SD_ReadBlock(u8 *buf, long long addr, u16 blksize)
{
    SD_Error errorstatus = SD_OK;
    u8 power;
    u32 count = 0, *tempbuff = (u32 *)buf; //ת��Ϊu32ָ��
    u32 timeout = SDIO_DATATIMEOUT;
    if (NULL == buf)
        return SD_INVALID_PARAMETER;
    SDIO->DCTRL = 0x0; //���ݿ��ƼĴ�������(��DMA)

    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //��������
    {
        blksize = 512;
        addr >>= 9;
    }
    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b; //���DPSM״̬������
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED; //������
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);

        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD16+�������ݳ���Ϊblksize,����Ӧ

        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); //�ȴ�R1��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����
    }
    else
        return SD_INVALID_PARAMETER;

    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; //���DPSM״̬������
    SDIO_DataInitStructure.SDIO_DataLength = blksize;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    SDIO_CmdInitStructure.SDIO_Argument = addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure); //����CMD17+��addr��ַ����ȡ����,����Ӧ

    errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK); //�ȴ�R1��Ӧ
    if (errorstatus != SD_OK)
        return errorstatus;            //��Ӧ����
    if (DeviceMode == SD_POLLING_MODE) //��ѯģʽ,��ѯ����
    {
        INTX_DISABLE(); //�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
        while (
            !(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9)))) //������/CRC/��ʱ/���(��־)/��ʼλ����
        {
            if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET) //����������,��ʾ���ٴ���8����
            {
                for (count = 0; count < 8; count++) //ѭ����ȡ����
                {
                    *(tempbuff + count) = SDIO->FIFO;
                }
                tempbuff += 8;
                timeout = 0X7FFFFF; //���������ʱ��
            }
            else //������ʱ
            {
                if (timeout == 0)
                    return SD_DATA_TIMEOUT;
                timeout--;
            }
        }
        if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
        {
            SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //������־
            return SD_DATA_TIMEOUT;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //���ݿ�CRC����
        {
            SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
            return SD_DATA_CRC_FAIL;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) //����fifo�������
        {
            SDIO_ClearFlag(SDIO_FLAG_RXOVERR); //������־
            return SD_RX_OVERRUN;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //������ʼλ����
        {
            SDIO_ClearFlag(SDIO_FLAG_STBITERR); //������־
            return SD_START_BIT_ERR;
        }
        while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) // FIFO����,�����ڿ�������
        {
            *tempbuff = SDIO->FIFO; //ѭ����ȡ����
            tempbuff++;
        }
        INTX_ENABLE();                     //�������ж�
        SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    }
    else if (DeviceMode == SD_DMA_MODE)
    {
        TransferError = SD_OK;
        StopCondition = 0; //�����,����Ҫ����ֹͣ����ָ��
        TransferEnd = 0;   //�����������λ�����жϷ�����1
        SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9); //������Ҫ���ж�
        SDIO->DCTRL |= 1 << 3;                                              // SDIO DMAʹ��
        SD_DMA_Config((u32 *)buf, blksize, DMA_DIR_PeripheralToMemory);
        while (((DMA2->LISR & (1 << 27)) == RESET) && (TransferEnd == 0) && (TransferError == SD_OK) && timeout)
            timeout--; //�ȴ��������
        if (timeout == 0)
            return SD_DATA_TIMEOUT; //��ʱ
        if (TransferError != SD_OK)
            errorstatus = TransferError;
    }
    return errorstatus;
}
#pragma *pack *(4)
u32 *tempbuff;
SD_Error SD_ReadMultiBlocks(u8 *buf, long long addr, u16 blksize, u32 nblks)
{
    SD_Error errorstatus = SD_OK;
    u8 power;
    u32 count = 0;
    u32 timeout = SDIO_DATATIMEOUT;
    tempbuff = (u32 *)buf;

    SDIO->DCTRL = 0x0;                          //���ݿ��ƼĴ�������(��DMA)
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //��������
    {
        blksize = 512;
        addr >>= 9;
    }

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED; //������
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);

        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

        if (errorstatus != SD_OK)
            return errorstatus;
    }
    else
        return SD_INVALID_PARAMETER;

    if (nblks > 1)
    {
        if (nblks * blksize > SD_MAX_DATA_LENGTH)
            return SD_INVALID_PARAMETER; //�ж��Ƿ񳬹������ճ���

        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig(&SDIO_DataInitStructure);

        SDIO_CmdInitStructure.SDIO_Argument = addr;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

        if (errorstatus != SD_OK)
            return errorstatus;

        if (DeviceMode == SD_POLLING_MODE)
        {
            INTX_DISABLE();
            while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 8) | (1 << 9))))
            {
                if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
                {
                    for (count = 0; count < 8; count++)
                    {
                        *(tempbuff + count) = SDIO->FIFO;
                    }
                    tempbuff += 8;
                    timeout = 0X7FFFFF;
                }
                else
                {
                    if (timeout == 0)
                        return SD_DATA_TIMEOUT;
                    timeout--;
                }
            }
            if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
            {
                SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //������־
                return SD_DATA_TIMEOUT;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //���ݿ�CRC����
            {
                SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
                return SD_DATA_CRC_FAIL;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) //����fifo�������
            {
                SDIO_ClearFlag(SDIO_FLAG_RXOVERR); //������־
                return SD_RX_OVERRUN;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //������ʼλ����
            {
                SDIO_ClearFlag(SDIO_FLAG_STBITERR); //������־
                return SD_START_BIT_ERR;
            }

            while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) // FIFO����,�����ڿ�������
            {
                *tempbuff = SDIO->FIFO; //ѭ����ȡ����
                tempbuff++;
            }
            if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) //���ս���
            {
                if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
                    (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
                {
                    SDIO_CmdInitStructure.SDIO_Argument = 0; //����CMD12+��������
                    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                    SDIO_SendCommand(&SDIO_CmdInitStructure);

                    errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION); //�ȴ�R1��Ӧ

                    if (errorstatus != SD_OK)
                        return errorstatus;
                }
            }
            INTX_ENABLE();                     //�������ж�
            SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
        }
        else if (DeviceMode == SD_DMA_MODE)
        {
            TransferError = SD_OK;
            StopCondition = 1; //����,��Ҫ����ֹͣ����ָ��
            TransferEnd = 0;   //�����������λ�����жϷ�����1
            SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9); //������Ҫ���ж�
            SDIO->DCTRL |= 1 << 3;                                              // SDIO DMAʹ��
            SD_DMA_Config((u32 *)buf, nblks * blksize, DMA_DIR_PeripheralToMemory);
            while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
                timeout--; //�ȴ��������
            if (timeout == 0)
                return SD_DATA_TIMEOUT; //��ʱ
            while ((TransferEnd == 0) && (TransferError == SD_OK))
                ;
            if (TransferError != SD_OK)
                errorstatus = TransferError;
        }
    }
    return errorstatus;
}
// SD��д1����
// buf:���ݻ�����
// addr:д��ַ
// blksize:���С
//����ֵ:����״̬
SD_Error SD_WriteBlock(u8 *buf, long long addr, u16 blksize)
{
    SD_Error errorstatus = SD_OK;

    u8 power = 0, cardstate = 0;

    u32 timeout = 0, bytestransferred = 0;

    u32 cardstatus = 0, count = 0, restwords = 0;

    u32 tlen = blksize; //�ܳ���(�ֽ�)

    u32 *tempbuff = (u32 *)buf;

    if (buf == NULL)
        return SD_INVALID_PARAMETER; //��������

    SDIO->DCTRL = 0x0; //���ݿ��ƼĴ�������(��DMA)

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    ; //���DPSM״̬������
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;           //������
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //��������
    {
        blksize = 512;
        addr >>= 9;
    }
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);

        SDIO_CmdInitStructure.SDIO_Argument = blksize; //����CMD16+�������ݳ���Ϊblksize,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); //�ȴ�R1��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����
    }
    else
        return SD_INVALID_PARAMETER;

    SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; //����CMD13,��ѯ����״̬,����Ӧ
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); //�ȴ�R1��Ӧ

    if (errorstatus != SD_OK)
        return errorstatus;
    cardstatus = SDIO->RESP1;
    timeout = SD_DATATIMEOUT;
    while (((cardstatus & 0x00000100) == 0) && (timeout > 0)) //���READY_FOR_DATAλ�Ƿ���λ
    {
        timeout--;

        SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; //����CMD13,��ѯ����״̬,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); //�ȴ�R1��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus;

        cardstatus = SDIO->RESP1;
    }
    if (timeout == 0)
        return SD_ERROR;

    SDIO_CmdInitStructure.SDIO_Argument = addr; //����CMD24,д����ָ��,����Ӧ
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK); //�ȴ�R1��Ӧ

    if (errorstatus != SD_OK)
        return errorstatus;

    StopCondition = 0; //����д,����Ҫ����ֹͣ����ָ��

    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
    ; // blksize, ����������
    SDIO_DataInitStructure.SDIO_DataLength = blksize;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    timeout = SDIO_DATATIMEOUT;

    if (DeviceMode == SD_POLLING_MODE)
    {
        INTX_DISABLE(); //�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
        while (
            !(SDIO->STA & ((1 << 10) | (1 << 4) | (1 << 1) | (1 << 3) | (1 << 9)))) //���ݿ鷢�ͳɹ�/����/CRC/��ʱ/��ʼλ����
        {
            if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) //���������,��ʾ���ٴ���8����
            {
                if ((tlen - bytestransferred) < SD_HALFFIFOBYTES) //����32�ֽ���
                {
                    restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4)
                                                                     : ((tlen - bytestransferred) / 4 + 1);

                    for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
                    {
                        SDIO->FIFO = *tempbuff;
                    }
                }
                else
                {
                    for (count = 0; count < 8; count++)
                    {
                        SDIO->FIFO = *(tempbuff + count);
                    }
                    tempbuff += 8;
                    bytestransferred += 32;
                }
                timeout = 0X3FFFFFFF; //д�������ʱ��
            }
            else
            {
                if (timeout == 0)
                    return SD_DATA_TIMEOUT;
                timeout--;
            }
        }
        if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
        {
            SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //������־
            return SD_DATA_TIMEOUT;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //���ݿ�CRC����
        {
            SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
            return SD_DATA_CRC_FAIL;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) //����fifo�������
        {
            SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); //������־
            return SD_TX_UNDERRUN;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //������ʼλ����
        {
            SDIO_ClearFlag(SDIO_FLAG_STBITERR); //������־
            return SD_START_BIT_ERR;
        }

        INTX_ENABLE();                     //�������ж�
        SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    }
    else if (DeviceMode == SD_DMA_MODE)
    {
        TransferError = SD_OK;
        StopCondition = 0; //����д,����Ҫ����ֹͣ����ָ��
        TransferEnd = 0;   //�����������λ�����жϷ�����1
        SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9); //���ò������ݽ�������ж�
        SD_DMA_Config((u32 *)buf, blksize, DMA_DIR_MemoryToPeripheral); // SDIO DMA����
        SDIO->DCTRL |= 1 << 3;                                          // SDIO DMAʹ��.
        while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
            timeout--; //�ȴ��������
        if (timeout == 0)
        {
            SD_Init(); //���³�ʼ��SD��,���Խ��д������������
            return SD_DATA_TIMEOUT; //��ʱ
        }
        timeout = SDIO_DATATIMEOUT;
        while ((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
            timeout--;
        if (timeout == 0)
            return SD_DATA_TIMEOUT; //��ʱ
        if (TransferError != SD_OK)
            return TransferError;
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    errorstatus = IsCardProgramming(&cardstate);
    while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
    {
        errorstatus = IsCardProgramming(&cardstate);
    }
    return errorstatus;
}
// SD��д�����
// buf:���ݻ�����
// addr:д��ַ
// blksize:���С
// nblks:Ҫд��Ŀ���
//����ֵ:����״̬
SD_Error SD_WriteMultiBlocks(u8 *buf, long long addr, u16 blksize, u32 nblks)
{
    SD_Error errorstatus = SD_OK;
    u8 power = 0, cardstate = 0;
    u32 timeout = 0, bytestransferred = 0;
    u32 count = 0, restwords = 0;
    u32 tlen = nblks * blksize; //�ܳ���(�ֽ�)
    u32 *tempbuff = (u32 *)buf;
    if (buf == NULL)
        return SD_INVALID_PARAMETER; //��������
    SDIO->DCTRL = 0x0;               //���ݿ��ƼĴ�������(��DMA)

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    ; //���DPSM״̬������
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;           //������
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //��������
    {
        blksize = 512;
        addr >>= 9;
    }
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);

        SDIO_CmdInitStructure.SDIO_Argument = blksize; //����CMD16+�������ݳ���Ϊblksize,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); //�ȴ�R1��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus; //��Ӧ����
    }
    else
        return SD_INVALID_PARAMETER;
    if (nblks > 1)
    {
        if (nblks * blksize > SD_MAX_DATA_LENGTH)
            return SD_INVALID_PARAMETER;
        if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
            (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
        {
            //�������
            SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; //����ACMD55,����Ӧ
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp1Error(SD_CMD_APP_CMD); //�ȴ�R1��Ӧ

            if (errorstatus != SD_OK)
                return errorstatus;

            SDIO_CmdInitStructure.SDIO_Argument = nblks; //����CMD23,���ÿ�����,����Ӧ
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT); //�ȴ�R1��Ӧ

            if (errorstatus != SD_OK)
                return errorstatus;
        }

        SDIO_CmdInitStructure.SDIO_Argument = addr; //����CMD25,���дָ��,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK); //�ȴ�R1��Ӧ

        if (errorstatus != SD_OK)
            return errorstatus;

        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
        ; // blksize, ����������
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig(&SDIO_DataInitStructure);

        if (DeviceMode == SD_POLLING_MODE)
        {
            timeout = SDIO_DATATIMEOUT;
            INTX_DISABLE(); //�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
            while (
                !(SDIO->STA & ((1 << 4) | (1 << 1) | (1 << 8) | (1 << 3) | (1 << 9)))) //����/CRC/���ݽ���/��ʱ/��ʼλ����
            {
                if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) //���������,��ʾ���ٴ���8��(32�ֽ�)
                {
                    if ((tlen - bytestransferred) < SD_HALFFIFOBYTES) //����32�ֽ���
                    {
                        restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4)
                                                                         : ((tlen - bytestransferred) / 4 + 1);
                        for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
                        {
                            SDIO->FIFO = *tempbuff;
                        }
                    }
                    else //���������,���Է�������8��(32�ֽ�)����
                    {
                        for (count = 0; count < SD_HALFFIFO; count++)
                        {
                            SDIO->FIFO = *(tempbuff + count);
                        }
                        tempbuff += SD_HALFFIFO;
                        bytestransferred += SD_HALFFIFOBYTES;
                    }
                    timeout = 0X3FFFFFFF; //д�������ʱ��
                }
                else
                {
                    if (timeout == 0)
                        return SD_DATA_TIMEOUT;
                    timeout--;
                }
            }
            if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
            {
                SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //������־
                return SD_DATA_TIMEOUT;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //���ݿ�CRC����
            {
                SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
                return SD_DATA_CRC_FAIL;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) //����fifo�������
            {
                SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); //������־
                return SD_TX_UNDERRUN;
            }
            else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //������ʼλ����
            {
                SDIO_ClearFlag(SDIO_FLAG_STBITERR); //������־
                return SD_START_BIT_ERR;
            }

            if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) //���ͽ���
            {
                if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
                    (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
                {
                    SDIO_CmdInitStructure.SDIO_Argument = 0; //����CMD12+��������
                    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                    SDIO_SendCommand(&SDIO_CmdInitStructure);

                    errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION); //�ȴ�R1��Ӧ
                    if (errorstatus != SD_OK)
                        return errorstatus;
                }
            }
            INTX_ENABLE();                     //�������ж�
            SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
        }
        else if (DeviceMode == SD_DMA_MODE)
        {
            TransferError = SD_OK;
            StopCondition = 1; //���д,��Ҫ����ֹͣ����ָ��
            TransferEnd = 0;   //�����������λ�����жϷ�����1
            SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9); //���ò������ݽ�������ж�
            SD_DMA_Config((u32 *)buf, nblks * blksize, DMA_DIR_MemoryToPeripheral); // SDIO DMA����
            SDIO->DCTRL |= 1 << 3;                                                  // SDIO DMAʹ��.
            timeout = SDIO_DATATIMEOUT;
            while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
                timeout--;    //�ȴ��������
            if (timeout == 0) //��ʱ
            {
                SD_Init(); //���³�ʼ��SD��,���Խ��д������������
                return SD_DATA_TIMEOUT; //��ʱ
            }
            timeout = SDIO_DATATIMEOUT;
            while ((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
                timeout--;
            if (timeout == 0)
                return SD_DATA_TIMEOUT; //��ʱ
            if (TransferError != SD_OK)
                return TransferError;
        }
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    errorstatus = IsCardProgramming(&cardstate);
    while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
    {
        errorstatus = IsCardProgramming(&cardstate);
    }
    return errorstatus;
}
// SDIO�жϷ�����
void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc(); //��������SDIO����ж�
}
// SDIO�жϴ�������
//����SDIO��������еĸ����ж�����
//����ֵ:�������
SD_Error SD_ProcessIRQSrc(void)
{
    if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) //��������ж�
    {
        if (StopCondition == 1)
        {
            SDIO_CmdInitStructure.SDIO_Argument = 0; //����CMD12+��������
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
        }
        else
            TransferError = SD_OK;
        SDIO->ICR |= 1 << 8; //�������жϱ��
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferEnd = 1;
        return (TransferError);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //����CRC����
    {
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferError = SD_DATA_CRC_FAIL;
        return (SD_DATA_CRC_FAIL);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
    {
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //���жϱ�־
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferError = SD_DATA_TIMEOUT;
        return (SD_DATA_TIMEOUT);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) // FIFO�������
    {
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR); //���жϱ�־
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferError = SD_RX_OVERRUN;
        return (SD_RX_OVERRUN);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) // FIFO�������
    {
        SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); //���жϱ�־
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferError = SD_TX_UNDERRUN;
        return (SD_TX_UNDERRUN);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //��ʼλ����
    {
        SDIO_ClearFlag(SDIO_FLAG_STBITERR); //���жϱ�־
        SDIO->MASK &=
            ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); //�ر�����ж�
        TransferError = SD_START_BIT_ERR;
        return (SD_START_BIT_ERR);
    }
    return (SD_OK);
}

//���CMD0��ִ��״̬
//����ֵ:sd��������
SD_Error CmdError(void)
{
    SD_Error errorstatus = SD_OK;
    u32 timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        if (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)
            break; //�����ѷ���(������Ӧ)
    }
    if (timeout == 0)
        return SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    return errorstatus;
}
//���R7��Ӧ�Ĵ���״̬
//����ֵ:sd��������
SD_Error CmdResp7Error(void)
{
    SD_Error errorstatus = SD_OK;
    u32 status;
    u32 timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        status = SDIO->STA;
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break; // CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    }
    if ((timeout == 0) || (status & (1 << 2))) //��Ӧ��ʱ
    {
        errorstatus = SD_CMD_RSP_TIMEOUT; //��ǰ������2.0���ݿ�,���߲�֧���趨�ĵ�ѹ��Χ
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������Ӧ��ʱ��־
        return errorstatus;
    }
    if (status & 1 << 6) //�ɹ����յ���Ӧ
    {
        errorstatus = SD_OK;
        SDIO_ClearFlag(SDIO_FLAG_CMDREND); //�����Ӧ��־
    }
    return errorstatus;
}
//���R1��Ӧ�Ĵ���״̬
// cmd:��ǰ����
//����ֵ:sd��������
SD_Error CmdResp1Error(u8 cmd)
{
    u32 status;
    while (1)
    {
        status = SDIO->STA;
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break; // CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) //��Ӧ��ʱ
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������Ӧ��ʱ��־
        return SD_CMD_RSP_TIMEOUT;
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC����
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); //�����־
        return SD_CMD_CRC_FAIL;
    }
    if (SDIO->RESPCMD != cmd)
        return SD_ILLEGAL_CMD;                         //���ƥ��
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);                 //������б��
    return (SD_Error)(SDIO->RESP1 & SD_OCR_ERRORBITS); //���ؿ���Ӧ
}
//���R3��Ӧ�Ĵ���״̬
//����ֵ:����״̬
SD_Error CmdResp3Error(void)
{
    u32 status;
    while (1)
    {
        status = SDIO->STA;
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break; // CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) //��Ӧ��ʱ
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������Ӧ��ʱ��־
        return SD_CMD_RSP_TIMEOUT;
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    return SD_OK;
}
//���R2��Ӧ�Ĵ���״̬
//����ֵ:����״̬
SD_Error CmdResp2Error(void)
{
    SD_Error errorstatus = SD_OK;
    u32 status;
    u32 timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        status = SDIO->STA;
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break; // CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    }
    if ((timeout == 0) || (status & (1 << 2))) //��Ӧ��ʱ
    {
        errorstatus = SD_CMD_RSP_TIMEOUT;
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������Ӧ��ʱ��־
        return errorstatus;
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC����
    {
        errorstatus = SD_CMD_CRC_FAIL;
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); //�����Ӧ��־
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    return errorstatus;
}
//���R6��Ӧ�Ĵ���״̬
// cmd:֮ǰ���͵�����
// prca:�����ص�RCA��ַ
//����ֵ:����״̬
SD_Error CmdResp6Error(u8 cmd, u16 *prca)
{
    SD_Error errorstatus = SD_OK;
    u32 status;
    u32 rspr1;
    while (1)
    {
        status = SDIO->STA;
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break; // CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) //��Ӧ��ʱ
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������Ӧ��ʱ��־
        return SD_CMD_RSP_TIMEOUT;
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC����
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); //�����Ӧ��־
        return SD_CMD_CRC_FAIL;
    }
    if (SDIO->RESPCMD != cmd) //�ж��Ƿ���Ӧcmd����
    {
        return SD_ILLEGAL_CMD;
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    rspr1 = SDIO->RESP1;               //�õ���Ӧ
    if (SD_ALLZERO == (rspr1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
    {
        *prca = (u16)(rspr1 >> 16); //����16λ�õ�,rca
        return errorstatus;
    }
    if (rspr1 & SD_R6_GENERAL_UNKNOWN_ERROR)
        return SD_GENERAL_UNKNOWN_ERROR;
    if (rspr1 & SD_R6_ILLEGAL_CMD)
        return SD_ILLEGAL_CMD;
    if (rspr1 & SD_R6_COM_CRC_FAILED)
        return SD_COM_CRC_FAILED;
    return errorstatus;
}

// SDIOʹ�ܿ�����ģʽ
// enx:0,��ʹ��;1,ʹ��;
//����ֵ:����״̬
SD_Error SDEnWideBus(u8 enx)
{
    SD_Error errorstatus = SD_OK;
    u32 scr[2] = {0, 0};
    u8 arg = 0X00;
    if (enx)
        arg = 0X02;
    else
        arg = 0X00;
    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED; // SD������LOCKED״̬
    errorstatus = FindSCR(RCA, scr);  //�õ�SCR�Ĵ�������
    if (errorstatus != SD_OK)
        return errorstatus;
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO) //֧�ֿ�����
    {
        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; //����CMD55+RCA,����Ӧ
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

        if (errorstatus != SD_OK)
            return errorstatus;

        SDIO_CmdInitStructure.SDIO_Argument = arg; //����ACMD6,����Ӧ,����:10,4λ;00,1λ.
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

        return errorstatus;
    }
    else
        return SD_REQUEST_NOT_APPLICABLE; //��֧�ֿ���������
}
//��鿨�Ƿ�����ִ��д����
// pstatus:��ǰ״̬.
//����ֵ:�������
SD_Error IsCardProgramming(u8 *pstatus)
{
    vu32 respR1 = 0, status = 0;

    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; //����Ե�ַ����
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;  //����CMD13
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    status = SDIO->STA;

    while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))))
        status = SDIO->STA;                              //�ȴ��������
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC���ʧ��
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); //���������
        return SD_CMD_CRC_FAIL;
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) //���ʱ
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); //���������
        return SD_CMD_RSP_TIMEOUT;
    }
    if (SDIO->RESPCMD != SD_CMD_SEND_STATUS)
        return SD_ILLEGAL_CMD;
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    respR1 = SDIO->RESP1;
    *pstatus = (u8)((respR1 >> 9) & 0x0000000F);
    return SD_OK;
}
//��ȡ��ǰ��״̬
// pcardstatus:��״̬
//����ֵ:�������
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
    SD_Error errorstatus = SD_OK;
    if (pcardstatus == NULL)
    {
        errorstatus = SD_INVALID_PARAMETER;
        return errorstatus;
    }

    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; //����CMD13,����Ӧ
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); //��ѯ��Ӧ״̬
    if (errorstatus != SD_OK)
        return errorstatus;
    *pcardstatus = SDIO->RESP1; //��ȡ��Ӧֵ
    return errorstatus;
}
//����SD����״̬
//����ֵ:SD��״̬
SDCardState SD_GetState(void)
{
    u32 resp1 = 0;
    if (SD_SendStatus(&resp1) != SD_OK)
        return SD_CARD_ERROR;
    else
        return (SDCardState)((resp1 >> 9) & 0x0F);
}
//����SD����SCR�Ĵ���ֵ
// rca:����Ե�ַ
// pscr:���ݻ�����(�洢SCR����)
//����ֵ:����״̬
SD_Error FindSCR(u16 rca, u32 *pscr)
{
    u32 index = 0;
    SD_Error errorstatus = SD_OK;
    u32 tempscr[2] = {0, 0};

    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8; //����CMD16,����Ӧ,����Block SizeΪ8�ֽ�
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN; //	 cmd16
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r1
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (errorstatus != SD_OK)
        return errorstatus;

    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD; //����CMD55,����Ӧ
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
    if (errorstatus != SD_OK)
        return errorstatus;

    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DataLength = 8; // 8���ֽڳ���,blockΪ8�ֽ�,SD����SDIO.
    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b; //���С8byte
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    SDIO_CmdInitStructure.SDIO_Argument = 0x0;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR; //����ACMD51,����Ӧ,����Ϊ0
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;    // r1
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
    if (errorstatus != SD_OK)
        return errorstatus;
    while (!(SDIO->STA &
             (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
    {
        if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) //����FIFO���ݿ���
        {
            *(tempscr + index) = SDIO->FIFO; //��ȡFIFO����
            index++;
            if (index >= 2)
                break;
        }
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //���ݳ�ʱ����
    {
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //������־
        return SD_DATA_TIMEOUT;
    }
    else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //���ݿ�CRC����
    {
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //������־
        return SD_DATA_CRC_FAIL;
    }
    else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) //����fifo�������
    {
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR); //������־
        return SD_RX_OVERRUN;
    }
    else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //������ʼλ����
    {
        SDIO_ClearFlag(SDIO_FLAG_STBITERR); //������־
        return SD_START_BIT_ERR;
    }
    SDIO_ClearFlag(SDIO_STATIC_FLAGS); //������б��
    //������˳��8λΪ��λ������.
    *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) |
                  ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
    *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) |
              ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
    return errorstatus;
}
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes)
{
    u8 count = 0;
    while (NumberOfBytes != 1)
    {
        NumberOfBytes >>= 1;
        count++;
    }
    return count;
}

void SD_DMA_Config(u32 *mbuf, u32 bufsize, u32 dir)
{

    DMA_InitTypeDef DMA_InitStructure;

    while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
    {
    }

    DMA_DeInit(DMA2_Stream3);

    DMA_InitStructure.DMA_Channel = DMA_Channel_4;                          //ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SDIO->FIFO;            // DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)mbuf;                      // DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = dir;                                        //�洢��������ģʽ
    DMA_InitStructure.DMA_BufferSize = 0;                                   //���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //�洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //�������ݳ���:32λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;         //�洢�����ݳ���:32λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ʹ����ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 //������ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;                   // FIFOʹ��
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           //ȫFIFO
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;               //����ͻ��4�δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;       //�洢��ͻ��4�δ���
    DMA_Init(DMA2_Stream3, &DMA_InitStructure);                             //��ʼ��DMA Stream

    DMA_FlowControllerConfig(DMA2_Stream3, DMA_FlowCtrl_Peripheral); //����������

    DMA_Cmd(DMA2_Stream3, ENABLE); //����DMA����
}

//��SD��
// buf:�����ݻ�����
// sector:������ַ
// cnt:��������
//����ֵ:����״̬;0,����;����,�������;
u8 SD_ReadDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 sta = SD_OK;
    long long lsector = sector;
    u8 n;
    lsector <<= 9;
    if ((u32)buf % 4 != 0)
    {
        for (n = 0; n < cnt; n++)
        {
            sta = SD_ReadBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512); //����sector�Ķ�����
            memcpy(buf, SDIO_DATA_BUFFER, 512);
            buf += 512;
        }
    }
    else
    {
        if (cnt == 1)
            sta = SD_ReadBlock(buf, lsector, 512); //����sector�Ķ�����
        else
            sta = SD_ReadMultiBlocks(buf, lsector, 512, cnt); //���sector
    }
    return sta;
}
//дSD��
// buf:д���ݻ�����
// sector:������ַ
// cnt:��������
//����ֵ:����״̬;0,����;����,�������;
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 sta = SD_OK;
    u8 n;
    long long lsector = sector;
    lsector <<= 9;
    if ((u32)buf % 4 != 0)
    {
        for (n = 0; n < cnt; n++)
        {
            memcpy(SDIO_DATA_BUFFER, buf, 512);
            sta = SD_WriteBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512); //����sector��д����
            buf += 512;
        }
    }
    else
    {
        if (cnt == 1)
            sta = SD_WriteBlock(buf, lsector, 512); //����sector��д����
        else
            sta = SD_WriteMultiBlocks(buf, lsector, 512, cnt); //���sector
    }
    return sta;
}
