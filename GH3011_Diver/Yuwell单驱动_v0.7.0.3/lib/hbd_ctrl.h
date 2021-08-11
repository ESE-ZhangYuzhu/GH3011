/*
*File Name: hbd_ctrl.h
************************************************************************
*
* Copyright 2018 - 2020 All rights reserved. Goodix Inc.
*
* Description: GH30x sensor control library header.
* Note:    
* Identifier abbreviation:  Wnd - Window
*                           Msg - Message        
**********************************************************************/

#ifndef __HBD_CTRL_H__
#define __HBD_CTRL_H__

typedef unsigned char GU8;
typedef signed char GS8;
typedef unsigned short GU16;
typedef signed short GS16;

typedef long int GS32;
typedef unsigned long int GU32;
typedef signed long long GS64;
typedef float GF32;
typedef unsigned char GBOOL;
#define BYTE_TRUE       (GU8)1
#define BYTE_FALSE      (GU8)0

/* Hbd functiional state enum. */
typedef enum 
{
    HBD_FUNCTIONAL_STATE_DISABLE = 0, 
    HBD_FUNCTIONAL_STATE_ENABLE = 1,
} EM_HBD_FUNCTIONAL_STATE;

/* Int status enum. */
enum EM_INT_STATUS
{
    INT_STATUS_CHIP_RESET = 0,
    INT_STATUS_NEW_DATA,
    INT_STATUS_FIFO_WATERMARK,
    INT_STATUS_FIFO_FULL,
    INT_STATUS_WEAR_DETECTED,
    INT_STATUS_UNWEAR_DETECTED,
    INT_STATUS_INVALID,
} ;

/* I2c Low Two bit select enum. */
enum EM_HBD_I2C_ID_SEL
{
    HBD_I2C_ID_SEL_1L0L = 0,
    HBD_I2C_ID_SEL_1L0H,
    HBD_I2C_ID_SEL_1H0L,
    HBD_I2C_ID_SEL_1H0H,
    HBD_I2C_ID_INVALID,
};

/* Led pd select enum. */
typedef enum 
{
    HBD_LED_PD_SEL_INTERNAL = 0,
    HBD_LED_PD_SEL_EXTERNAL,
} EM_HBD_LED_PD_SEL;

/* Led logic channel map to  led enum. */
typedef enum
{
    HBD_LED_LOGIC_CHANNEL_MAP_PHY012 = 0,
    HBD_LED_LOGIC_CHANNEL_MAP_PHY021,
    HBD_LED_LOGIC_CHANNEL_MAP_PHY102,
    HBD_LED_LOGIC_CHANNEL_MAP_PHY120,
    HBD_LED_LOGIC_CHANNEL_MAP_PHY201,
    HBD_LED_LOGIC_CHANNEL_MAP_PHY210,
} EM_HBD_LED_LOGIC_CHANNEL_MAP;

/* sample time enum. */
typedef enum 
{
    HBD_SAMPLE_TIME_32CYCLES = 0,
    HBD_SAMPLE_TIME_64CYCLES,
    HBD_SAMPLE_TIME_128CYCLES,
    HBD_SAMPLE_TIME_256CYCLES,
    HBD_SAMPLE_TIME_512CYCLES,
    HBD_SAMPLE_TIME_1024CYCLES,
    HBD_SAMPLE_TIME_2048CYCLES,
} EM_HBD_SAMPLE_TIME;

/* tia gain enum. */
typedef enum
{
    HBD_TIA_GAIN_0 = 0,
    HBD_TIA_GAIN_1,
    HBD_TIA_GAIN_2,
    HBD_TIA_GAIN_3,
    HBD_TIA_GAIN_4,
    HBD_TIA_GAIN_5,
    HBD_TIA_GAIN_6,
    HBD_TIA_GAIN_7,
} EM_HBD_TIA_GAIN;

/* Adt config struct type. Notes: only logic channel0 & channel1 valid in adt mode. */
typedef struct
{
    EM_HBD_FUNCTIONAL_STATE emGINTEnable;                  // GINT enable flag
    EM_HBD_LED_LOGIC_CHANNEL_MAP emLedLogicChannelMap;     // Logic map
    EM_HBD_LED_PD_SEL emLogicChannel0PDSelect;             // Logic Channel0 PD select
    EM_HBD_LED_PD_SEL emLogicChannel1PDSelect;             // Logic Channel1 PD select
    GU8 uchLogicChannel0Current;                           // Logic Channel0 current cofig(1 step = 400uA)
    GU8 uchLogicChannel1Current;                           // Logic Channel1 current cofig(1 step = 400uA)
    EM_HBD_TIA_GAIN emLogicChannel0TiaGain;                // Logic Channel0 tia gain(valid value:0-7)
    EM_HBD_TIA_GAIN emLogicChannel1TiaGain;                // Logic Channel1 tia gain(valid value:0-7)
    EM_HBD_SAMPLE_TIME emSampleTime;                       // Sample time
} ST_ADT_CONFIG_TYPE;

/* Hbd init config struct type. */
typedef struct
{
    ST_ADT_CONFIG_TYPE stAdtInitConfig;
} ST_HBD_INIT_CONFIG_TYPE;

/* register struct. */
typedef struct  
{
    GU16 usRegAddr;
    GU16 usRegData;
} ST_REGISTER;



/* Hbd function return code definitions list. */
#define HBD_RET_OK                          (0)                /**< There is no error */
#define HBD_RET_GENERIC_ERROR               (-1)               /**< A generic error happens */
#define HBD_RET_PARAMETER_ERROR             (-2)               /**< Parameter error */
#define HBD_RET_COMM_NOT_REGISTERED_ERROR   (-3)               /**< I2c/Spi communication interface not registered error */
#define HBD_RET_COMM_ERROR                  (-4)               /**< I2c/Spi Communication error */
#define HBD_RET_RESOURCE_ERROR              (-5)               /**< Resource full or not available error */
#define HBD_RET_NO_INITED_ERROR             (-6)               /**< No inited error */
#define HBD_RET_LED_CONFIG_ALL_OFF_ERROR    (-7)               /**< Led config all off error */

/******* hbd init default config ****************                        
 * stAdtInitConfig = { HBD_FUNCTIONAL_STATE_DISABLE,        /// GINT disable
 *                     HBD_LED_LOGIC_CHANNEL_MAP_PHY012,    /// Logic map
 *                     HBD_LED_PD_SEL_INTERNAL,             /// Logic Channel0 PD select
 *                     HBD_LED_PD_SEL_INTERNAL,             /// Logic Channel1 PD select
 *                     0x19,                                /// Logic Channel0 current 10mA (1 step = 400uA)
 *                     0x19,                                /// Logic Channel1 current 10mA (1 step = 400uA)
 *                     HBD_TIA_GAIN_3,                      /// Logic Channel0 tia gain
 *                     HBD_TIA_GAIN_3,                      /// Logic Channel1 tia gain
 *                     HBD_SAMPLE_TIME_128CYCLES,           /// sample time
 *                   }
 */
#define HBD_INIT_CONFIG_DEFAULT_DEF(var) ST_HBD_INIT_CONFIG_TYPE var={\
                                                                      {HBD_FUNCTIONAL_STATE_DISABLE,\
                                                                      HBD_LED_LOGIC_CHANNEL_MAP_PHY012, \
                                                                      HBD_LED_PD_SEL_INTERNAL, \
                                                                      HBD_LED_PD_SEL_INTERNAL,\
                                                                      0x19, 0x19, HBD_TIA_GAIN_3, HBD_TIA_GAIN_3, HBD_SAMPLE_TIME_128CYCLES}\
                                                                    }
 
                                                                 
/****************************************************************
* Description: set i2c operation function
* Input:  uchI2cIdLowTwoBitsSelect: i2c low two bits addr selected, see EM_HBD_I2C_ID_SEL
          pI2cWriteFunc: i2c write function pointer
          pI2cReadFunc: i2c read function pointer
* Return: HBD_RET_PARAMETER_ERROR: function pointer parameter set null error,
          HBD_RET_OK: register success
******************************************************************/
GS8 HBD_SetI2cRW(GU8 uchI2cIdLowTwoBitsSelect,
                   GU8 (*pI2cWriteFunc)(GU8 uchDeviceId, const GU8 uchWriteBytesArr[], GU16 usWriteLen),  
                   GU8 (*pI2cReadFunc)(GU8 uchDeviceId, const GU8 uchCmddBytesArr[], GU16 usCmddLen, GU8 uchReadBytesArr[], GU16 usMaxReadLen));

/****************************************************************
* Description: set delay function callback
* Input:  pDelayUsFunc:delay function callback
* Return: None
******************************************************************/
void HBD_SetDelayUsCallback(void (*pDelayUsFunc)(GU16 usUsec));

/****************************************************************
* Description: load new GH30x reg config array
* Input:    stRegConfigArr: reg config array ,
            usRegConfigLen: reg config array len, 
* Return: HBD_RET_OK:load config success, HBD_RET_COMM_ERROR: load config error,
******************************************************************/
GS8 HBD_LoadNewRegConfigArr(const ST_REGISTER stRegConfigArr[], GU16 usRegConfigLen);

/*****************************************************************************************
* Description: set LED current
* Input:  fLed0Current:LED0 current
          fLed1Current:LED1 current
* Return: HBD_RET_PARAMETER_ERROR: out of range,and set current to the closest limit value
          HBD_RET_GENERIC_ERROR: can't set current in this mode
          HBD_RET_OK: set current success
******************************************************************************************/
GS8 HBD_SetLedCurrent(GF32 fLed0Current, GF32 fLed1Current);

/****************************************************************
* Description: get LED current
* output:  pLed0Current:ptr of LED0 current
          pLed1Current:ptr of LED1 current
* Return: HBD_RET_GENERIC_ERROR: can't get current in this mode
          HBD_RET_OK: get current success
******************************************************************/
GS8 HBD_GetLedCurrrent(GF32 *pLed0Current, GF32 *pLed1Current);

/****************************************************************
* Description: Communication operation interface confirm.
* Output:  None
* Return: HBD_RET_OK=GH30x comunicate ok, 
          HBD_RET_COMM_ERROR=GH30x comunicate error,
          HBD_RET_COMM_NOT_REGISTERED_ERROR=GH30x i2c/spi interface not registered,
******************************************************************/
GS8 HBD_CommunicationInterfaceConfirm(void);

/****************************************************************
* Description: simple init hbd configure parameters
* Input:    stHbInitConfigParam: Init Config struct ,see ST_HBD_INIT_CONFIG_TYPE,
* Return: HBD_RET_OK=success, 
          HBD_RET_PARAMETER_ERROR=paramters error,
          HBD_RET_COMM_ERROR=GH30x comunicate error, 
          HBD_RET_COMM_NOT_REGISTERED_ERROR=GH30x i2c interface not registered
******************************************************************/
GS8 HBD_SimpleInit(ST_HBD_INIT_CONFIG_TYPE *stHbdInitConfigParam);

/****************************************************************
* Description: stop hbd
* Input:  None
* Return: HBD_RET_OK=success, 
          HBD_RET_NO_INITED_ERROR=fail:don't init success 
******************************************************************/
GS8 HBD_Stop(void);

/****************************************************************
* Description: start adt wear detect
* Input:  None
* Return: HBD_RET_OK=success, 
          HBD_RET_NO_INITED_ERROR=fail:don't init success 
******************************************************************/
GS8 HBD_AdtWearDetectStart(void);

/****************************************************************
* Description: start adt wear detect continuous
* Input:  None
* Return: HBD_RET_OK=success, 
          HBD_RET_NO_INITED_ERROR=fail:don't init success 
******************************************************************/
GS8 HBD_AdtWearContinuousDetectStart(void);

/****************************************************************
* Description: check adt has started
* Input:  None
* Return: 1= has started, 
          0= has not started
******************************************************************/
GU8 HBD_IsAdtWearDetectHasStarted(void);

/****************************************************************
* Description: get Int status 
* Input:  None
* Return: Status, see EM_INT_STATUS
******************************************************************/
GU8 HBD_GetIntStatus(void);

/****************************************************************
* Description: get fifo count has read
* Input:  None,  
* Return: fifo count has read, 
******************************************************************/
GU8 HBD_GetFifoCntHasRead(void);

/****************************************************************
* Description: Reset chip
* Input:  None
* Return: HBD_RET_OK=success, 
          HBD_RET_COMM_NOT_REGISTERED_ERROR=GH30x i2c interface not registered
******************************************************************/
GS8 HBD_ChipReset(void);

/****************************************************************
* Description: set irq pluse width 
* Input:  uchIrqPluseWidth: irq width(us) setting ,
                            if set 0: fixed 1us and return HBD_RET_PARAMETER_ERROR
* Return: HBD_RET_OK=success, 
          HBD_RET_PARAMETER_ERROR=fail:parameter error
******************************************************************/
GS8 HBD_SetIrqPluseWidth(GU8 uchIrqPluseWidth);

/****************************************************************
* Description: Get version
* Input:    None,
* Return: library Hbd ctrl version
******************************************************************/
GS8 * HBD_GetHbdVersion(void);

/****************************************************************
* Description: Get hbd lib compile time
* Input:    None,
* Return: hbd lib compile time string
******************************************************************/
GS8 * HBD_GetHbdCompileTime(void);

/****************************************************************
* Description: Start HBD mode for get rawdata
* Input:  GU16 usSampleRate : sample rate
*         GU8 ucEnableFifo : 1 enable fifo, 0 disable fifo
*         GU16 usFifoTHR : Fifo int generate threshold
* Return: 0 success, others fail
******************************************************************/
GS8 HBD_StartHBDOnly(GU16 usSampleRate, GU8 ucEnableFifo, GU16 usFifoTHR);

/****************************************************************
* Description: Get rawdata in fifo int
* Input:  GU16 usBufLen : rawdata array length(frames), if usBufLen < *pusRealDataLen value, data will be lost
* Output: GS32 nRawdataOut: ptr of rawdata array output
* Return: 0 OK, 1 soft autoled error
******************************************************************/
GU8 HBD_GetRawdataByFifoInt(GU16 usBufLen, GS32 nRawdataOut[][2], GU16 *pusRealDataLen);

/****************************************************************
* Description: Get rawdata in new data int
* Output: GU32 *ppg1 
*         GU32 *ppg2
* Return: 0 OK, 1 soft autoled error
******************************************************************/
GU8 HBD_GetRawdataByNewDataInt(GU32 *ppg1, GU32 *ppg2);

/*****************************************************************************************
* Description: must be called after HBD_GetRawdataByFifoInt or HBD_GetRawdataByNewDataInt
* 
*****************************************************************************************/
void HBD_GetRawdataHasDone(void);

#endif /* __HBD_CTRL_H__ */

/********END OF FILE********* (C) COPYRIGHT 2018 .********/
