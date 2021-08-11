/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-10 14:26:56
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-11 13:40:32
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
        reinit_ret = HBD_SimpleInit(&gh30x_init_config);
        reinit_cnt--;
    } while (reinit_ret != HBD_RET_OK);
    if (reinit_ret == HBD_RET_OK) // if reinit ok, restart last mode
    {
#if (__USE_GOODIX_APP__)
        if (goodix_app_start_app_mode)
        {
            SEND_GH30X_RESET_EVT();
        }
        else
#endif
        {
            gh30x_start_adt_with_mode();
    EXAMPLE_DEBUG_LOG_L1("got gh30x reset evt, reinit [%s]\r\n", dbg_ret_val_string[DEBUG_HBD_RET_VAL_BASE + reinit_ret]);
}

/// gh30x unwear  evt handler
void gh30x_unwear_evt_handler(void)
{
#if (__USE_GOODIX_APP__)
    if (goodix_app_start_app_mode)
    {
        SEND_AUTOLED_FAIL_EVT();
        EXAMPLE_DEBUG_LOG_L1("got gh30x unwear evt, restart func\r\n");
        gh30x_start_func_whithout_adt_confirm();
    }
    else
#endif
    {
        gh30x_start_adt_with_mode();
        EXAMPLE_DEBUG_LOG_L1("got gh30x unwear evt, start adt detect\r\n");
        SEND_MCU_HB_MODE_WEAR_STATUS(WEAR_STATUS_UNWEAR, NULL, 0);
        HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP();
        handle_wear_status_result(WEAR_STATUS_UNWEAR);
    }
}

/// gh30x wear evt handler
void gh30x_wear_evt_handler(void)
{
    gh30x_start_func_with_mode();
    EXAMPLE_DEBUG_LOG_L1("got gh30x wear evt\r\n");
    SEND_MCU_HB_MODE_WEAR_STATUS(WEAR_STATUS_WEAR, NULL, 0);
    handle_wear_status_result(WEAR_STATUS_WEAR);
}

/// calc unwear status handle
void gh30x_handle_calc_unwear_status(void)
{
    gh30x_start_adt_with_mode();
    EXAMPLE_DEBUG_LOG_L1("calc unwear status, start adt detect\r\n");
    handle_wear_status_result(WEAR_STATUS_UNWEAR);
}

/// fifo evt getrawdata mode
static void gh30x_fifo_evt_getrawdata_mode_only(void)
{
    GU16 realdatalen = 0;
    GS32 rawdata[__GET_RAWDATA_BUF_LEN__][2] = {0};
    GU8 currentarr[3] = {0};
#if (__USE_GOODIX_APP__)
    GU16 current = 0;
    current = HBD_I2cReadReg(0x0122);
    currentarr[0] = current & 0xFF;
    currentarr[1] = (current >> 8) & 0xFF;
    current = HBD_I2cReadReg(0x0124);
    currentarr[2] = current & 0xFF;
#else
    (void)currentarr;
#endif
    GU8 nRes = 0;
    nRes = HBD_GetRawdataByFifoInt(__GET_RAWDATA_BUF_LEN__, rawdata, &realdatalen);
    HBD_GetRawdataHasDone();

    GU8 wearing_state = WEAR_STATUS_WEAR;
    if (1 == nRes)
    {
        wearing_state = WEAR_STATUS_UNWEAR;
    }
    handle_getrawdata_mode_result(wearing_state, rawdata, realdatalen);
    EXAMPLE_DEBUG_LOG_L1("get rawdata only %d, rawdata_len=%d\r\n", nRes, realdatalen);

    if (wearing_state == WEAR_STATUS_UNWEAR)
    {
        SEND_MCU_HB_MODE_RESULT(WEAR_STATUS_UNWEAR, currentarr, (GS32(*)[DBG_MCU_MODE_PKG_LEN])dbg_rawdata_ptr, realdatalen);
        HBD_Stop();
        gh30x_handle_calc_unwear_status();
    }
    else
    {
        HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_START();
        SEND_MCU_HB_MODE_RESULT(WEAR_STATUS_WEAR, currentarr, (GS32(*)[DBG_MCU_MODE_PKG_LEN])dbg_rawdata_ptr, realdatalen);
    }
    EXAMPLE_LOG_RAWDARA("get rawdata only:\r\n", dbg_rawdata_ptr, dbg_rawdata_len);
}

/// gh30x fifo evt handler
void gh30x_fifo_evt_handler(void)
{
    gh30x_fifo_evt_getrawdata_mode_only();
    EXAMPLE_DEBUG_LOG_L1("got gh30x fifo evt\r\n");
}

/// gh30x newdata evt handler
void gh30x_new_data_evt_handler(void)
{
    EXAMPLE_DEBUG_LOG_L1("got gh30x new data evt\r\n");
    GU32 PPG1, PPG2;
    GU8 nRes = HBD_GetRawdataByNewDataInt(&PPG1, &PPG2);
    GU8 wearing_state = WEAR_STATUS_WEAR;
    if (1 == nRes)
    {
        wearing_state = WEAR_STATUS_UNWEAR;
    }
    GS32 rawdata[1][DBG_MCU_MODE_PKG_LEN];
    rawdata[0][0] = PPG1;
    rawdata[0][1] = PPG2;
    handle_getrawdata_mode_result(wearing_state, (GS32(*)[2])rawdata, 1);
    HBD_GetRawdataHasDone();
    if (wearing_state == WEAR_STATUS_UNWEAR)
    {
        SEND_MCU_HB_MODE_WEAR_STATUS(WEAR_STATUS_UNWEAR, rawdata, 1);
        HBD_Stop();
        gh30x_handle_calc_unwear_status();
    }
    else
    {
        HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_START();
        SEND_MCU_HB_MODE_WEAR_STATUS(WEAR_STATUS_WEAR, rawdata, 1);
    }
}

/// gh30x fifo full evt handler
void gh30x_fifo_full_evt_handler(void)
{
    HBD_Stop();
    gh30x_start_func_with_mode();
    EXAMPLE_DEBUG_LOG_L1("got gh30x fifo full evt, shouldn't reach here!!\r\n");
}

/// gh30x int msg handler
void gh30x_int_msg_handler(void)
{
    GU8 gh30x_irq_status;
    GU8 gh30x_adt_working_flag;
    gh30x_irq_status = HBD_GetIntStatus();
    gh30x_adt_working_flag = HBD_IsAdtWearDetectHasStarted();

    if (gh30x_irq_status == INT_STATUS_FIFO_WATERMARK)
    {
        gh30x_fifo_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_NEW_DATA)
    {
        gh30x_new_data_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_WEAR_DETECTED)
    {
        gh30x_wear_evt_handler();
    }
    else if (gh30x_irq_status == INT_STATUS_UNWEAR_DETECTED)
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
        gh30x_start_adt_with_mode();
    }
}
