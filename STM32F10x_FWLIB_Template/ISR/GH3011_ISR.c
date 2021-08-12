/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-10 14:26:56
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-12 16:51:35
 * @FilePath: \STM32F10x_FWLIB_Template\ISR\GH3011_ISR.c
 * @Description: 
 */
#include "GH3011_ISR.h"

/// gh30x reset evt handler
void gh30x_reset_evt_handler(void)
{
    GS8 reinit_ret = HBD_RET_OK;
    GU8 reinit_cnt = __RESET_REINIT_CNT_CONFIG__;
    // reinit
    do
    {
        reinit_ret = HBD_SimpleInit(&GH3011_InitStruct); // 初始化GH3011
        reinit_cnt--;
    } while (reinit_ret != HBD_RET_OK);
    if (reinit_ret == HBD_RET_OK)                                                // if reinit ok, restart last mode
        gh30x_adt_wear_detect_start(gh30x_reg_config_ptr, gh30x_reg_config_len); // 初始化成功后开启佩戴检测
}

/// gh30x unwear  evt handler
void gh30x_unwear_evt_handler(void) // 如果检测到佩戴失败，就再次检测
{
    gh30x_adt_wear_detect_start(gh30x_reg_config_ptr, gh30x_reg_config_len);
    EXAMPLE_DEBUG_LOG_L1("got gh30x unwear evt, start adt detect\r\n");
}

/// gh30x wear evt handler
void gh30x_wear_evt_handler(void) // 如果检测到佩戴成功，就进行数据采集
{
    gh30x_getrawdata_start();
    EXAMPLE_DEBUG_LOG_L1("got gh30x wear evt\r\n");
}

/// calc unwear status handle
void gh30x_handle_calc_unwear_status(void)
{
    gh30x_adt_wear_detect_start(gh30x_reg_config_ptr, gh30x_reg_config_len);
    EXAMPLE_DEBUG_LOG_L1("calc unwear status, start adt detect\r\n");
}

/// gh30x fifo evt handler
void gh30x_fifo_evt_handler(void)
{
    EXAMPLE_DEBUG_LOG_L1("got gh30x fifo evt\r\n");
    GU16 realdatalen = 0;
    GS32 rawdata[__GET_RAWDATA_BUF_LEN__][2] = {0};
    GU8 nRes = 0;
    nRes = HBD_GetRawdataByFifoInt(__GET_RAWDATA_BUF_LEN__, rawdata, &realdatalen);
    HBD_GetRawdataHasDone();
}

/// gh30x newdata evt handler
void gh30x_new_data_evt_handler(void)
{
    EXAMPLE_DEBUG_LOG_L1("got gh30x new data evt\r\n");
    GU32 PPG1, PPG2;
    GU8 nRes = HBD_GetRawdataByNewDataInt(&PPG1, &PPG2);

    HBD_GetRawdataHasDone();
}

/// gh30x fifo full evt handler
void gh30x_fifo_full_evt_handler(void)
{
    HBD_Stop();
    gh30x_getrawdata_start();
    EXAMPLE_DEBUG_LOG_L1("got gh30x fifo full evt, shouldn't reach here!!\r\n");
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
        gh30x_adt_wear_detect_start(gh30x_reg_config_ptr, gh30x_reg_config_len);
    }
}
