/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-10 14:26:56
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-27 09:29:01
 * @FilePath: \STM32F10x_FWLIB_Template\ISR\GH3011_ISR.c
 * @Description: 
 */
#include "GH3011_ISR.h"
#include "UART.h"

/// GH3011 reset Event Handler
static void GH3011_Reset_Event_Handler(void)
{
    GS8 Reinit_Ret = HBD_RET_OK;
    GU8 Reinit_Cnt = __RESET_REINIT_CNT_CONFIG__;

    GH3011_DEBUG_LOG("\n\r GH3011 Reset Hander!");

    do
    {
        Reinit_Ret = HBD_SimpleInit(&GH3011.InitStruct); // 初始化GH3011
        if ((Reinit_Cnt--) == 0)
        {
            GH3011_DEBUG_LOG("\n\r GH3011 reset Handler error!");
            break;
        }
    } while (Reinit_Ret != HBD_RET_OK);

    if (Reinit_Ret == HBD_RET_OK)
    {
        GH3011_DEBUG_LOG("\n\r GH3011 reset and Init success!"); // if reinit ok, restart last mode
        GH3011_ADT_WeraDetect_Start(&GH3011);                    // 初始化成功后开启佩戴检测
    }
}

/// GH3011 unwear  Event Handler
static void GH3011_Unwear_Event_Handler(void) // 如果在测量过程中检测到佩戴失败，就立即停止
{
    GH3011_DEBUG_LOG("\n\r GH3011 Got Unwear Event, STOP Detecting!");
    HBD_Stop();
}

/// GH3011 wear Event Handler
static void GH3011_Wear_Event_Handler(void) // 如果检测到佩戴成功，就进行数据采集
{
    GH3011_DEBUG_LOG("\n\r Got GH3011 Wear Event, get raw data start");
    GH3011_GetRawData_Start(&GH3011);
}

// GH3011 FIFO Event Handler
// 一帧数据中 包含2个PPG数据 PPG0 是绿光数据 PPG1是红外光数据
static void GH3011_FIFO_Event_Handler(void)
{
    // GH3011_DEBUG_LOG("\n\r Got GH3011 FIFO Event");
    GU16 Real_DataLength = 0;
    GS32 Raw_Data[__GET_RAWDATA_BUF_LEN__][2] = {0};

    if (HBD_RET_OK == HBD_GetRawdataByFifoInt(__GET_RAWDATA_BUF_LEN__, Raw_Data, &Real_DataLength))
    {
        u8 Raw_Data_Index = 0;
        while (Real_DataLength--)
        {
            GH3011.GH3011_RawData[Raw_Data_Index].RawData[0] = Raw_Data[Raw_Data_Index][0]; // PPG0
            GH3011.GH3011_RawData[Raw_Data_Index].RawData[1] = Raw_Data[Raw_Data_Index][1]; // PPG1
            printf("\n\r %08X\t%08X", GH3011.GH3011_RawData[Raw_Data_Index].RawData[0], GH3011.GH3011_RawData[Raw_Data_Index].RawData[1]);
            Raw_Data_Index++;
        }
        // GH3011_DEBUG_LOG("\n\r Get Raw Data By FIFO Success!");
    }
    else
    {
        GH3011_DEBUG_LOG("\n\r Get Raw Data By FIFO Failed!");
    }

    HBD_GetRawdataHasDone();
}

/// GH3011 NewdDta Event Handler
static void GH3011_New_Data_Event_Handler(void)
{
    GH3011_DEBUG_LOG("\n\r Got GH3011 New Data Event");
    GU32 PPG1, PPG2;
    if (HBD_RET_OK == HBD_GetRawdataByNewDataInt(&PPG1, &PPG2))
        GH3011_DEBUG_LOG("\n\r Get Raw Data By New Success!");
    else
        GH3011_DEBUG_LOG("\n\r Get Raw Data By New Success!");

    HBD_GetRawdataHasDone();
}

/// GH3011 FIFO Full Event Handler
static void GH3011_FIFO_Full_Event_Handler(void)
{
    HBD_Stop();
    GH3011_DEBUG_LOG("Got GH3011 FIFO Full Event, shouldn't reach here!!\r\n");
    HBD_ChipReset();
}

/// GH3011 Handler
void GH3011_Handler(void)
{
    GU8 GH3011_IRQ_Status;
    GU8 GH3011_ADT_Working_Flag;
    GH3011_IRQ_Status = HBD_GetIntStatus();
    GH3011_ADT_Working_Flag = HBD_IsAdtWearDetectHasStarted();

    if (GH3011_IRQ_Status == INT_STATUS_FIFO_WATERMARK) // FIFO 阈值
    {
        GH3011_FIFO_Event_Handler();
    }
    else if (GH3011_IRQ_Status == INT_STATUS_NEW_DATA) // 新数据就绪
    {
        GH3011_New_Data_Event_Handler();
    }
    else if (GH3011_IRQ_Status == INT_STATUS_WEAR_DETECTED) // 佩戴成功
    {
        GH3011_Wear_Event_Handler();
    }
    else if (GH3011_IRQ_Status == INT_STATUS_UNWEAR_DETECTED) // 佩戴失败，脱落
    {
        GH3011_Unwear_Event_Handler();
    }
    else if (GH3011_IRQ_Status == INT_STATUS_CHIP_RESET) // if GH3011 reset, need reinit
    {
        GH3011_Reset_Event_Handler();
    }
    else if (GH3011_IRQ_Status == INT_STATUS_FIFO_FULL) // if GH3011 fifo full, need restart
    {
        GH3011_FIFO_Full_Event_Handler();
    }

    if ((GH3011_ADT_Working_Flag == 1) && (GH3011_IRQ_Status != INT_STATUS_WEAR_DETECTED) && (GH3011_IRQ_Status != INT_STATUS_UNWEAR_DETECTED)) // adt working
    {
        // ADT 已经启动 但是发生了意外的中断 佩戴和没有佩戴都没有检测到
        GH3011_ADT_WeraDetect_Start(&GH3011);
    }
}
