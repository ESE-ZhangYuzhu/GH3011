/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-10 14:26:56
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-13 16:48:03
 * @FilePath: \STM32F10x_FWLIB_Template\ISR\GH3011_ISR.c
 * @Description: 
 */
#include "GH3011_ISR.h"

/// gh30x reset evt handler
static void gh30x_reset_evt_handler(void)
{
    GS8 reinit_ret = HBD_RET_OK;
    GU8 reinit_cnt = __RESET_REINIT_CNT_CONFIG__;

    GH3011_DEBUG_LOG("\n\r GH3011 reset hander!");

    do
    {
        reinit_ret = HBD_SimpleInit(&GH3011.InitStruct); // 初始化GH3011
        if ((reinit_cnt--) == 0)
        {
            GH3011_DEBUG_LOG("\n\r GH3011 reset handler error!");
            break;
        }
    } while (reinit_ret != HBD_RET_OK);

    if (reinit_ret == HBD_RET_OK)
    {
        GH3011_DEBUG_LOG("\n\r GH3011 reset and Init success!"); // if reinit ok, restart last mode
        GH3011_ADT_WeraDetect_Start(&GH3011);                    // 初始化成功后开启佩戴检测
    }
}

/// gh30x unwear  evt handler
static void gh30x_unwear_evt_handler(void) // 如果检测到佩戴失败，就再次检测
{
    static u8 GH3011_Unwear_Counter = 0;
    GH3011_Unwear_Counter++;

    if (GH3011_Unwear_Counter < 5)
    {
        GH3011_DEBUG_LOG("\n\r got gh30x unwear evt, start adt detect");
        GH3011_ADT_WeraDetect_Start(&GH3011);
    }
    else
    {
        GH3011_Unwear_Counter = 0;
        GH3011_DEBUG_LOG("\n\r GH3011 got too many unwear evt, STOP Detecting!");
        HBD_Stop();
    }
}

/// gh30x wear evt handler
static void gh30x_wear_evt_handler(void) // 如果检测到佩戴成功，就进行数据采集
{
    GH3011_DEBUG_LOG("\n\r got gh30x wear evt, get raw data start");
    GH3011_GetRawData_Start(&GH3011);
}

/// gh30x fifo evt handler
static void gh30x_fifo_evt_handler(void)
{
    GH3011_DEBUG_LOG("\n\r got gh30x fifo evt");
    GU16 realdatalen = 0;
    GS32 rawdata[__GET_RAWDATA_BUF_LEN__][2] = {0};
    GU8 nRes = 0;
    nRes = HBD_GetRawdataByFifoInt(__GET_RAWDATA_BUF_LEN__, rawdata, &realdatalen);
    GH3011_DEBUG_LOG("\n\r Get Raw data By FIFO result %d", nRes);

    GH3011_DEBUG_LOG("\n\rGreen PPG Raw Data:");
    for (u8 i = 0; i < realdatalen; i++)
    {
        GH3011_DEBUG_LOG("\t%X", rawdata[i][0]);
    }

    GH3011_DEBUG_LOG("\n\rIR PPG Raw Data :");
    for (u8 j = 0; j < realdatalen; j++)
    {

        GH3011_DEBUG_LOG("\t%X", rawdata[j][1]);
    }
    HBD_GetRawdataHasDone();
}

/// gh30x newdata evt handler
static void gh30x_new_data_evt_handler(void)
{
    GH3011_DEBUG_LOG("\n\r got gh30x new data evt");
    GU32 PPG1, PPG2;
    GU8 nRes = HBD_GetRawdataByNewDataInt(&PPG1, &PPG2);
    GH3011_DEBUG_LOG("\n\r Get Raw data By New data result %d", nRes);
    HBD_GetRawdataHasDone();
}

/// gh30x fifo full evt handler
static void gh30x_fifo_full_evt_handler(void)
{
    HBD_Stop();
    GH3011_DEBUG_LOG("got gh30x fifo full evt, shouldn't reach here!!\r\n");
    HBD_ChipReset();
}

/// gh30x int msg handler
void gh30x_int_msg_handler(void)
{
    GU8 gh30x_irq_status;
    GU8 gh30x_adt_working_flag;
    gh30x_irq_status = HBD_GetIntStatus();
    gh30x_adt_working_flag = HBD_IsAdtWearDetectHasStarted();

    if (gh30x_irq_status == INT_STATUS_FIFO_WATERMARK) // FIFO 阈值
    {
        gh30x_fifo_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_NEW_DATA) // 新数据就绪
    {
        gh30x_new_data_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_WEAR_DETECTED) // 佩戴成功
    {
        gh30x_wear_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_UNWEAR_DETECTED) // 佩戴失败，脱落
    {
        gh30x_unwear_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_CHIP_RESET) // if gh30x reset, need reinit
    {
        gh30x_reset_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_FIFO_FULL) // if gh30x fifo full, need restart
    {
        gh30x_fifo_full_evt_handler();
    }

    if ((gh30x_adt_working_flag == 1) && (gh30x_irq_status != INT_STATUS_WEAR_DETECTED) && (gh30x_irq_status != INT_STATUS_UNWEAR_DETECTED)) // adt working
    {
        // adt 已经启动 但是发生了意外的中断 佩戴和没有佩戴都没有检测到
        GH3011_ADT_WeraDetect_Start(&GH3011);
    }
}
