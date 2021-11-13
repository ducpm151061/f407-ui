#include "as5600.h"
#include "delay.h"

u16 _rawStartAngle = 0;
u16 _zPosition = 0;
u16 _rawEndAngle = 0;
u16 _mPosition = 0;
u16 _maxAngle = 0;

void AS_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    AS_SCL_HIGH;
    AS_SDA_HIGH;
}

static u8 as_start(void)
{
    AS_OUT();
    AS_SDA_HIGH;
    AS_SCL_HIGH;
    delay_us(1);

    // if(!AS_SDA_STATE) return AS_BUS_BUSY;

    AS_SDA_LOW;
    delay_us(1);

    AS_SCL_LOW;
    delay_us(1);

    // if(AS_SDA_STATE) return AS_BUS_ERROR;

    return AS_READY;
}

static void as_stop(void)
{
    AS_OUT();
    AS_SCL_LOW;
    AS_SDA_LOW;
    delay_us(1);

    //	AS_SCL_LOW;
    //  delay_us(1);

    AS_SCL_HIGH;
    AS_SDA_HIGH;
    delay_us(1);
}

static unsigned char as_wait_ack(void)
{
    volatile unsigned char ucErrTime = 0;
    AS_IN();
    AS_SDA_HIGH;
    delay_us(1);
    AS_SCL_HIGH;
    delay_us(1);
    while (AS_SDA_STATE)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            as_stop();
            return 1;
        }
    }
    AS_SCL_LOW;
    return AS_READY;
}

static void as_send_ack(void)
{
    AS_SCL_LOW;
    AS_OUT();
    AS_SDA_LOW;
    delay_us(1);
    AS_SCL_HIGH;
    delay_us(1);
    AS_SCL_LOW;
    delay_us(1);
}

static void as_send_nack(void)
{
    AS_SCL_LOW;
    AS_OUT();
    AS_SDA_HIGH;
    delay_us(1);
    AS_SCL_HIGH;
    delay_us(1);
    AS_SCL_LOW;
    delay_us(1);
}

static u8 as_send_byte(u8 reg)
{
    u8 i;
    AS_OUT();
    AS_SCL_LOW;
    for (i = 0; i < 8; i++)
    {
        if (reg & 0x80)
            AS_SDA_HIGH;
        else
            AS_SDA_LOW;

        reg <<= 1;
        delay_us(1);

        AS_SCL_HIGH;
        delay_us(1);
        AS_SCL_LOW;
        delay_us(1);
    }
    return AS_READY;
}

static u8 as_receive_byte(void)
{
    u8 i, res;
    AS_IN();
    // AS_SDA_HIGH;
    // AS_SCL_LOW;
    res = 0;

    for (i = 0; i < 8; i++)
    {
        AS_SCL_LOW;
        delay_us(1);
        AS_SCL_HIGH;
        // delay_us(1);
        res <<= 1;

        if (AS_SDA_STATE)
            res |= 0x01;

        // AS_SCL_LOW;
        delay_us(1);
    }
    as_send_nack();
    return res;
}

static u8 as_receive_byte_with_ack(void)
{

    u8 i, res;
    AS_IN();
    // AS_SDA_HIGH;
    // AS_SCL_LOW;
    res = 0;

    for (i = 0; i < 8; i++)
    {
        AS_SCL_LOW;
        delay_us(1);
        AS_SCL_HIGH;
        // delay_us(1);
        res <<= 1;

        if (AS_SDA_STATE)
            res |= 0x01;

        // AS_SCL_LOW;
        delay_us(1);
    }
    as_send_ack();
    return res;
}

static u8 as_read8(u8 moni_dev_addr, u8 moni_reg_addr, u8 moni_i2c_len, u8 *moni_i2c_data_buf)
{

    as_start();
    as_send_byte(moni_dev_addr << 1 | AS_DIR_TRANS);
    as_wait_ack();
    as_send_byte(moni_reg_addr);
    as_wait_ack();
    // as_stop();

    as_start();
    as_send_byte(moni_dev_addr << 1 | AS_DIR_RECV);
    as_wait_ack();
    while (moni_i2c_len)
    {
        if (moni_i2c_len == 1)
            *moni_i2c_data_buf = as_receive_byte();
        else
            *moni_i2c_data_buf = as_receive_byte_with_ack();
        moni_i2c_data_buf++;
        moni_i2c_len--;
    }
    as_stop();
    return 0x00;
}

static int8_t as_write8(u8 moni_dev_addr, u8 moni_reg_addr, u8 moni_i2c_len, u8 *moni_i2c_data_buf)
{
    u8 i;
    as_start();
    as_send_byte(moni_dev_addr << 1 | AS_DIR_TRANS);
    as_wait_ack();
    as_send_byte(moni_reg_addr);
    as_wait_ack();

    // as_start();
    for (i = 0; i < moni_i2c_len; i++)
    {
        as_send_byte(moni_i2c_data_buf[i]);
        as_wait_ack();
    }
    as_stop();
    return 0;
}

static u8 high_byte(u16 value)
{
    u8 ret;
    value = value >> 8;
    ret = (u8)value;
    return ret;
}

static u8 low_byte(u16 value)
{
    u8 ret;
    value = value & 0x00ff;
    ret = (u8)value;
    return ret;
}
/*******************************************************
 Method: read_byte
 In: register to read
 Out: data read from i2c
 Description: reads one byte register from i2c
******************************************************/
static u8 read_byte(u8 in_adr)
{
    u8 ret = -1;

    as_read8(_ams5600_Address, in_adr, 1, &ret);
    delay_us(1);
    return ret;
}

/*******************************************************
 Method: readOnTwoByte
 In: two registers to read
 Out: data read from i2c as a int16_t
 Description: reads two bytes register from i2c
******************************************************/
static u16 read_2_bytes(u8 in_adr_hi, u8 in_adr_lo)
{
    u16 res = -1;
    u8 low = 0, high = 0;

    /* Read Low Byte */
    low = read_byte(in_adr_lo);

    /* Read High Byte */
    high = read_byte(in_adr_hi);

    // printf("high:%d,low:%d  ",high,low);
    res = high << 8;
    res = res | low;
    // printf("retVal:%d\r\n",retVal);
    return res;
}

/*******************************************************
 Method: write_byte
 In: address and data to write
 Out: none
 Description: writes one byte to a i2c register
******************************************************/
static void write_byte(u8 adr_in, u8 dat_in)
{
    u8 dat = dat_in;
    as_write8(_ams5600_Address, adr_in, 1, &dat);
}

/*******************************************************
 Method: get_max_angle
 In: none
 Out: value of max angle register
 Description: gets value of maximum angle register.
******************************************************/
int16_t get_max_angle(void)
{
    return read_2_bytes(_mang_hi, _mang_lo);
}

/*******************************************************
 Method: get_start_pos
 In: none
 Out: value of start position register
 Description: gets value of start position register.
******************************************************/
int16_t get_start_pos(void)
{
    return read_2_bytes(_zpos_hi, _zpos_lo);
}

/*******************************************************
 Method: gen_end_pos
 In: none
 Out: value of end position register
 Description: gets value of end position register.
******************************************************/
int16_t gen_end_pos(void)
{
    return read_2_bytes(_mpos_hi, _mpos_lo);
}

/*******************************************************
 Method: get_scaled_angle
 In: none
 Out: value of scaled angle register
 Description: gets scaled value of magnet position.
 start, end, or max angle settings are used to
 determine value
******************************************************/
int16_t get_scaled_angle(void)
{
    return read_2_bytes(_ang_hi, _ang_lo);
}

/*******************************************************
 Method: get Agc
 In: none
 Out: value of AGC register
 Description: gets value of AGC register.
******************************************************/
int16_t get_agc()
{
    return read_byte(_agc);
}

/*******************************************************
 Method: get_magnitude
 In: none
 Out: value of magnitude register
 Description: gets value of magnitude register.
******************************************************/
int16_t get_magnitude()
{
    return read_2_bytes(_mag_hi, _mag_lo);
}

/*******************************************************
 Method: get_burn_count
 In: none
 Out: value of zmco register
 Description: determines how many times chip has been
 permanently written to.
******************************************************/
int16_t get_burn_count()
{
    return read_byte(_zmco);
}
/*******************************************************
 Method: get_raw_angle
 In: none
 Out: value of raw angle register
 Description: gets raw value of magnet position.
 start, end, and max angle settings do not apply
******************************************************/
int16_t get_raw_angle(void)
{
    return read_2_bytes(_raw_ang_hi, _raw_ang_lo);
}
/*******************************************************
 Method: setEndtPosition
 In: new end angle position
 Out: value of end position register
 Description: sets a value in end position register.
 If no value is provided, method will read position of
 magnet.
******************************************************/
int16_t setEndPosition(int16_t endAngle)
{
    if (endAngle == -1)
        _rawEndAngle = get_raw_angle();
    else
        _rawEndAngle = endAngle;

    write_byte(_mpos_hi, high_byte(_rawEndAngle));
    delay_ms(2);
    write_byte(_mpos_lo, low_byte(_rawEndAngle));
    delay_ms(2);
    _mPosition = read_2_bytes(_mpos_hi, _mpos_lo);

    return (_mPosition);
}

/*******************************************************
 Method: setStartPosition
 In: new start angle position
 Out: value of start position register
 Description: sets a value in start position register.
 If no value is provided, method will read position of
 magnet.
******************************************************/
int16_t setStartPosition(int16_t startAngle)
{
    if (startAngle == -1)
    {
        _rawStartAngle = get_raw_angle();
    }
    else
        _rawStartAngle = startAngle;

    write_byte(_zpos_hi, high_byte(_rawStartAngle));
    delay_ms(2);
    write_byte(_zpos_lo, low_byte(_rawStartAngle));
    delay_ms(2);
    _zPosition = read_2_bytes(_zpos_hi, _zpos_lo);

    return (_zPosition);
}

/*******************************************************
 Method: set_max_angle
 In: new maximum angle to set OR none
 Out: value of max angle register
 Description: sets a value in maximum angle register.
 If no value is provided, method will read position of
 magnet.  Setting this register zeros out max position
 register.
******************************************************/
int16_t set_max_angle(int16_t newMaxAngle)
{
    int32_t res;
    if (newMaxAngle == -1)
    {
        _maxAngle = get_raw_angle();
    }
    else
        _maxAngle = newMaxAngle;

    write_byte(_mang_hi, high_byte(_maxAngle));
    delay_ms(2);
    write_byte(_mang_lo, low_byte(_maxAngle));
    delay_ms(2);

    res = read_2_bytes(_mang_hi, _mang_lo);
    return res;
}

/*******************************************************
 Method: detect_magnet
 In: none
 Out: 1 if magnet is detected, 0 if not
 Description: reads status register and examines the
 MH bit
******************************************************/
bool detect_magnet(void)
{
    u8 mag_status;
    u8 res = false;
    /*0 0 MD ML MH 0 0 0*/
    /* MD high = AGC minimum overflow, Magnet to strong */
    /* ML high = AGC Maximum overflow, magnet to weak*/
    /* MH high = magnet detected*/
    mag_status = read_byte(_stat);

    if (mag_status & 0x20)
        res = true;

    return res;
}

/*******************************************************
 Method: get_magnet_strength
 In: none
 Out: 0 if no magnet is detected
      1 if magnet is to weak
      2 if magnet is just right
      3 if magnet is to strong
 Description: reads status register andexamins the MH,ML,MD bits
******************************************************/
u8 get_magnet_strength(void)
{
    u8 mag_status;
    u8 res = 0;
    /*0 0 MD ML MH 0 0 0*/
    /* MD high = AGC minimum overflow, Magnet to strong */
    /* ML high = AGC Maximum overflow, magnet to weak*/
    /* MH high = magnet detected*/
    mag_status = read_byte(_stat);
    if (detect_magnet() == 1)
    {
        res = 2; /*just right */
        if (mag_status & 0x10)
            res = 1; /*to weak */
        else if (mag_status & 0x08)
            res = 3; /*to strong */
    }

    return res;
}

/*******************************************************
 Method: burn_angle
 In: none
 Out: 1 success
     -1 no magnet
     -2 burn limit exceeded
     -3 start and end positions not set (useless burn)
 Description: burns start and end positions to chip.
 THIS CAN ONLY BE DONE 3 TIMES
******************************************************/
u8 burn_angle()
{
    u8 res = 1;
    _zPosition = get_start_pos();
    _mPosition = gen_end_pos();
    _maxAngle = get_max_angle();

    if (detect_magnet() == 1)
    {
        if (get_burn_count() < 3)
        {
            if ((_zPosition == 0) && (_mPosition == 0))
                res = -3;
            else
                write_byte(_burn, 0x80);
        }
        else
            res = -2;
    }
    else
        res = -1;

    return res;
}

/*******************************************************
 Method: burn_max_angle_and_config
 In: none
 Out: 1 success
     -1 burn limit exceeded
     -2 max angle is to small, must be at or above 18 degrees
 Description: burns max angle and config data to chip.
 THIS CAN ONLY BE DONE 1 TIME
******************************************************/
u8 burn_max_angle_and_config()
{
    u8 res = 1;
    _maxAngle = get_max_angle();

    if (get_burn_count() == 0)
    {
        if (_maxAngle * 0.087 < 18)
            res = -2;
        else
            write_byte(_burn, 0x40);
    }
    else
        res = -1;

    return res;
}

/*******************************************************
 Function: convert_raw_angle_to_degrees
 In: angle data from AMS_5600::get_raw_angle
 Out: human readable degrees as float
 Description: takes the raw angle and calculates
 float value in degrees.
******************************************************/
static float convert_raw_angle_to_degrees(int16_t newAngle)
{
    /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
    float res = newAngle * 0.087;
    return res;
}

/*******************************************************
 Function: angle_to_degrees
 In: none
 Out: human readable degrees as float
 Description: takes the raw angle and calculates
 float value in degrees.
******************************************************/
int16_t get_angle(void)
{
    return convert_raw_angle_to_degrees(get_raw_angle());
}
