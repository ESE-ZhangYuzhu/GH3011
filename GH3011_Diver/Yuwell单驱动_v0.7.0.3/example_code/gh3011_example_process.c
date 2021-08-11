/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_process.c
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 *
 */

#include "gh3011_example_common.h"
#include "gh3011_example.h"
#include <math.h>

/// app mode start flag
bool goodix_app_start_app_mode = false;

/// system test mode start flag
uint8_t goodix_system_test_mode = 0;

/// system test os led num
uint8_t goodix_system_test_os_led_num = 0;

/// gh30x sample rate
uint16_t gh30x_SampleRate = 0;

/// gh30x fifo enable
uint8_t gh30x_EnableFifo = 0;

/// gh30x fifo threshold
uint16_t gh30x_FifoThreshold = 0;

const ST_REGISTER *gh30x_reg_config_ptr = NULL;
uint16_t gh30x_reg_config_len = 0;

#if (__USE_GOODIX_APP__)
GS32 dbg_rawdata_ptr[__GET_RAWDATA_BUF_LEN__ * DBG_MCU_MODE_PKG_LEN] = {0};
#endif

/// gh30x module init, include gsensor init
int gh30x_module_init(void)
{
    static bool first_init_flag = true;
    GS8 init_err_flag = HBD_RET_OK;

    /* log all version string */
    EXAMPLE_DEBUG_LOG_L1(GH30X_EXAMPLE_VER_STRING);
    EXAMPLE_DEBUG_LOG_L1("hbd ctrl lib version: %s\r\n", HBD_GetHbdVersion());
#if (__SYSTEM_TEST_SUPPORT__)
    EXAMPLE_DEBUG_LOG_L1("test lib version: %s\r\n", HBDTEST_Get_TestlibVersion());
#endif
    if (first_init_flag)
    {
#if (__GH30X_COMMUNICATION_INTERFACE__ == GH30X_COMMUNICATION_INTERFACE_SPI)
        hal_gh30x_spi_init();                                                                               // spi init
        HBD_SetI2cRW(HBD_I2C_ID_SEL_1L0L, gh30x_i2c_write_exchange_to_spi, gh30x_i2c_read_exchange_to_spi); // register i2c exchange to spi api
#else                                                                                                       // (__GH30X_COMMUNICATION_INTERFACE__ == GH30X_COMMUNICATION_INTERFACE_I2C)
        hal_gh30x_i2c_init();                                                       // i2c init
        HBD_SetI2cRW(HBD_I2C_ID_SEL_1L0L, hal_gh30x_i2c_write, hal_gh30x_i2c_read); // register i2c RW func api
#endif

#if (__PLATFORM_DELAY_US_CONFIG__)
        HBD_SetDelayUsCallback(hal_gh30x_delay_us);
#endif
    }

    init_err_flag = HBD_SimpleInit(&gh30x_init_config); // init gh30x
    if (HBD_RET_OK != init_err_flag)
    {
        EXAMPLE_DEBUG_LOG_L1("gh30x init error[%s]\r\n", dbg_ret_val_string[DEBUG_HBD_RET_VAL_BASE + init_err_flag]);
        return GH30X_EXAMPLE_ERR_VAL;
    }

    if (first_init_flag)
    {

        hal_gh30x_int_init(); // gh30x int pin init

#if (__GH30X_IRQ_PLUSE_WIDTH_CONFIG__)
        HBD_SetIrqPluseWidth(255); // set Irq pluse width (255us)
#endif

        gh30x_comm_pkg_init(); // comm pkg init

        HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_INIT();
    }

    EXAMPLE_DEBUG_LOG_L1("gh30x module init ok\r\n");
    first_init_flag = false;
    return GH30X_EXAMPLE_OK_VAL;
}

/// gh30x module config
void gh30x_module_config(ST_GH30X_CONFIG stGh30xConfig)
{
    gh30x_SampleRate = stGh30xConfig.SampleRate;
    gh30x_EnableFifo = stGh30xConfig.EnableFifo;
    gh30x_FifoThreshold = stGh30xConfig.FifoThreshold;
    gh30x_reg_config_ptr = stGh30xConfig.reg_config_ptr;
    gh30x_reg_config_len = stGh30xConfig.reg_config_len;
}

/// gh30x module start, with adt
void gh30x_module_start(void)
{
    gh30x_start_adt_with_mode();
    EXAMPLE_DEBUG_LOG_L1("gh30x module start\r\n");
}

/// gh30x module start, without adt
void gh30x_module_start_without_adt()
{
    gh30x_start_func_with_mode();
    EXAMPLE_DEBUG_LOG_L1("gh30x module start without adt\r\n");
}

/// gh30x module stop
void gh30x_module_stop(void)
{
    gh30x_stop_func();
    EXAMPLE_DEBUG_LOG_L1("gh30x module stop\r\n");
#if (__USE_GOODIX_APP__)
    goodix_app_start_app_mode = false; // if call stop, clear app mode
#endif
}

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
        }
    }
    EXAMPLE_DEBUG_LOG_L1("got gh30x reset evt, reinit [%s]\r\n", dbg_ret_val_string[DEBUG_HBD_RET_VAL_BASE + reinit_ret]);
}

/// gh30x unwear  evt handler
void gh30x_unwear_evt_handler(void)
{
    gh30x_start_adt_with_mode();
    EXAMPLE_DEBUG_LOG_L1("got gh30x unwear evt, start adt detect\r\n");
    SEND_MCU_HB_MODE_WEAR_STATUS(WEAR_STATUS_UNWEAR, NULL, 0);
    HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP();
    handle_wear_status_result(WEAR_STATUS_UNWEAR);
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
#if (__USE_GOODIX_APP__)
    static GU8 uchPackId = 0;

    for (GU16 i = 0; i < realdatalen; i++)
    {
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN] = rawdata[i][0];
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN + 1] = rawdata[i][1];
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN + 2] = 0;
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN + 3] = 0;
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN + 4] = 0;
        dbg_rawdata_ptr[i * DBG_MCU_MODE_PKG_LEN + 5] = uchPackId++;
    }
#endif
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
#if (__USE_GOODIX_APP__)
    if (goodix_app_start_app_mode)
    {
        EXAMPLE_DEBUG_LOG_L2("got gh30x new data evt, send rawdata to app\n");
        if (GH30X_AUTOLED_ERR_VAL == HBD_SendRawdataPackageByNewdataInt())
        {
            SEND_AUTOLED_FAIL_EVT();
        }
    }
    else
#endif
    {
#if (__USE_GOODIX_APP__)
        goodix_app_start_app_mode = false; // if call stop, clear app mode
#endif

#if (__SYSTEM_TEST_SUPPORT__)
        if (goodix_system_test_mode)
        {
            if (!ledmask[goodix_system_test_os_led_num])
            {
                if (goodix_system_test_os_led_num < 2)
                {
                    goodix_system_test_os_led_num++;
                    gh30x_systemtest_os_start(goodix_system_test_os_led_num);
                }
                else
                {
                    gh30x_systemtest_part2_handle(0);
                }
            }
            EXAMPLE_DEBUG_LOG_L1("got gh30x new data evt, put data to system test module.\r\n");
            GU8 os_test_ret = gh30x_systemtest_os_calc(goodix_system_test_os_led_num);
            if (os_test_ret != 0xFF) // test has done
            {
                EXAMPLE_DEBUG_LOG_L1("system test os[led %d] ret: %d!\r\n", goodix_system_test_os_led_num, os_test_ret);
                if (goodix_system_test_os_led_num < 2 && (!os_test_ret))
                {
                    goodix_system_test_os_led_num++;
                    if (goodix_system_test_os_led_num < 2 && 0 == ledmask[goodix_system_test_os_led_num])
                    {
                        goodix_system_test_os_led_num++;
                        if (0 == ledmask[goodix_system_test_os_led_num])
                        {
                            HBD_Stop();
                            goodix_system_test_os_led_num = goodix_system_test_os_led_num - 2;
                            gh30x_systemtest_part2_handle(os_test_ret);
                        }
                        else
                        {
                            gh30x_systemtest_os_start(goodix_system_test_os_led_num);
                            EXAMPLE_DEBUG_LOG_L1("system test change to test next led:%d!\r\n", goodix_system_test_os_led_num);
                        }
                    }
                    else if (2 == goodix_system_test_os_led_num && 0 == ledmask[goodix_system_test_os_led_num])
                    {
                        HBD_Stop();
                        goodix_system_test_os_led_num--;
                        gh30x_systemtest_part2_handle(os_test_ret);
                    }
                    else
                    {
                        gh30x_systemtest_os_start(goodix_system_test_os_led_num);
                        EXAMPLE_DEBUG_LOG_L1("system test change to test next led:%d!\r\n", goodix_system_test_os_led_num);
                    }
                }
                else
                {
                    //                    goodix_system_test_mode = false;
                    //                    EXAMPLE_DEBUG_LOG_L1("system test has done!\r\n");
                    HBD_Stop();
                    gh30x_systemtest_part2_handle(os_test_ret);
                }
            }
        }
        else
#endif
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
    HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP();
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

/// gh30x fifo int timeout msg handler
void gh30x_fifo_int_timeout_msg_handler(void)
{
    GU8 gh30x_irq_status_1;
    GU8 gh30x_irq_status_2;

    HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP();

    EXAMPLE_DEBUG_LOG_L1("fifo int time out!!!\r\n");

    gh30x_irq_status_1 = HBD_GetIntStatus();
    gh30x_irq_status_2 = HBD_GetIntStatus();
    if ((gh30x_irq_status_1 == INT_STATUS_FIFO_WATERMARK) && (gh30x_irq_status_2 == INT_STATUS_INVALID))
    {
        gh30x_fifo_evt_handler();
    }
    else
    {
        gh30x_reset_evt_handler();
    }
}

/// communicate parse handler
void gh30x_communicate_parse_handler(GS8 communicate_type, GU8 *buffer, GU8 length)
{
#if (__USE_GOODIX_APP__)
    EM_COMM_CMD_TYPE comm_cmd_type = HBD_CommParseHandler(communicate_type, buffer, length); // parse recv data
    if (communicate_type == COMM_TYPE_INVALID_VAL)
    {
        EXAMPLE_DEBUG_LOG_L1("comm_type error, pelase check inited or not, @ref gh30x_module_init!!!\r\n");
    }
    else
    {
        EXAMPLE_DEBUG_LOG_L1("parse: cmd[%x-%s], comm_type[%d], length[%d]\r\n", buffer[0], dbg_comm_cmd_string[(uint8_t)comm_cmd_type], communicate_type, length);
    }
    if (comm_cmd_type < COMM_CMD_INVALID)
    {
        handle_goodix_communicate_cmd(comm_cmd_type);
        ST_GH30X_CONFIG stGh30xConfig = {0};
        if (comm_cmd_type == COMM_CMD_ALGO_IN_APP_HB_START) // handle all app mode cmd
        {
            goodix_app_start_app_mode = true;
            stGh30xConfig.SampleRate = 25;
            stGh30xConfig.EnableFifo = HBD_FUNCTIONAL_STATE_DISABLE;
            gh30x_module_config(stGh30xConfig);
            gh30x_module_start_without_adt();
        }
        else if (comm_cmd_type == COMM_CMD_ADT_SINGLE_MODE_START) // handle mcu mode cmd
        {
            goodix_app_start_app_mode = false;

            stGh30xConfig.SampleRate = 25;
            stGh30xConfig.EnableFifo = HBD_FUNCTIONAL_STATE_ENABLE;
            stGh30xConfig.FifoThreshold = 25;
            stGh30xConfig.reg_config_ptr = getrawdata_reg_config_array;
            stGh30xConfig.reg_config_len = getrawdata_reg_config_array_len;
            gh30x_module_config(stGh30xConfig);
            gh30x_module_start();
        }
        else // handle all stop cmd
        {
            goodix_app_start_app_mode = false;
            gh30x_module_stop();
        }
    }
#endif
}

/// start gh30x adt func with adt_run_mode
void gh30x_start_adt_with_mode(void)
{
    gh30x_adt_wear_detect_start(gh30x_reg_config_ptr, gh30x_reg_config_len);
}

/// start gh30x func with func_run_mode
void gh30x_start_func_with_mode(void)
{
    gh30x_getrawdata_start();
}

/// stop gh30x func
void gh30x_stop_func(void)
{
    HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP();
    HBD_Stop();
#if (__HBD_USE_DYN_MEM__)
    gh30x_free_memory();
#endif
}

/// gh30x start func fix adt confirm
void gh30x_start_func_whithout_adt_confirm(void)
{
    gh30x_start_func_with_mode();
}

#if (__SYSTEM_TEST_SUPPORT__)
/// gh30x module system test os check
void gh30x_module_system_test_os_start(void)
{
    handle_before_system_os_test();
    gh30x_module_stop();
    goodix_system_test_os_led_num = 0;
    gh30x_systemtest_os_start(goodix_system_test_os_led_num);
    EXAMPLE_DEBUG_LOG_L1("system test os check start\r\n");
}
#endif

//gh30x module system test
void gh30x_systemtest_start(EMGh30xTestItem mode)
{
#if (__SYSTEM_TEST_SUPPORT__)
#if __PLATFORM_DELAY_US_CONFIG__
    EXAMPLE_DEBUG_LOG_L1("delay has been out");
    HBDTEST_set_delayFunc(&hal_gh30x_delay_us);
#endif
    uint8_t ret = 0;
    if (!mode)
    {
        handle_system_test_result(EN_PARAM_FAIL, 0);
        EXAMPLE_DEBUG_LOG_L1("system test check fail, ret = %d\r\n", EN_PARAM_FAIL);
    }
    if (mode & 0x1)
    {
        gh30x_module_stop();
        ret = gh30x_systemtest_comm_check();
        if (ret)
        {
            handle_system_test_result(ret, 0);
            return;
        }
        EXAMPLE_DEBUG_LOG_L1("system test comm check, ret = %d\r\n", ret);
    }
    if (mode & 0x2)
    {
        gh30x_module_stop();
        ret = gh30x_systemtest_otp_check();
        if (ret)
        {
            handle_system_test_result(ret, 0);
            return;
        }
        EXAMPLE_DEBUG_LOG_L1("system test otp check, ret = %d\r\n", ret);
    }
    if (mode & 0xc)
    {
        goodix_system_test_mode = (mode & 0x1c) >> 1;
        EXAMPLE_DEBUG_LOG_L1("begin goodix_system_test_mode is %d,mode is %d!\r\n", goodix_system_test_mode, mode);
        HBDTEST_ROMALEDCheckData *hbdatalst[] = {&led0std, &led1std, &led2std};
        for (int i = 0; i < 3; i++)
        {
            gh30x_systemtest_param_set(i, &(hbdatalst[i]->_param));
        }
        gh30x_module_system_test_os_start();
        EXAMPLE_DEBUG_LOG_L1("come to second part.\n");
    }
    else
    {
        handle_system_test_result(0, 0);
    }
#else
    EXAMPLE_DEBUG_LOG_L1("__SYSTEM_TEST_SUPPORT__ disabled in config\r\n");
#endif
}

#if (__SYSTEM_TEST_SUPPORT__)
void gh30x_systemtest_part2_handle(uint8_t ret)
{
    EXAMPLE_DEBUG_LOG_L1("now goodix_system_test_mode is %d!\r\n", goodix_system_test_mode);
    for (int16_t i = 0; i < 3; i++)
    {
        if (goodix_system_test_mode & (1 << i))
        {
            goodix_system_test_mode ^= (1 << i);
            if (i == 2 && goodix_system_test_mode & 0x8)
            {
                goodix_system_test_mode ^= (1 << 3);
            }
            break;
        }
    }
    EXAMPLE_DEBUG_LOG_L1("now goodix_system_test_mode is %d!\r\n", goodix_system_test_mode);
    if (goodix_system_test_mode && (!ret))
    {
        gh30x_module_system_test_os_start();
    }
    else
    {
        goodix_system_test_mode = 0;
        EXAMPLE_DEBUG_LOG_L1("system part2 test has done!\r\n");
        HBD_Stop();
        handle_system_test_result(ret, goodix_system_test_os_led_num);
    }
}
#endif

/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
