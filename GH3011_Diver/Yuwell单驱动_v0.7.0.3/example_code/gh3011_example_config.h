/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_config.h
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 *
 */

#ifndef _GH3011_EXAMPLE_CONFIG_H_
#define _GH3011_EXAMPLE_CONFIG_H_


// application config

/* common */

/// gh30x communicate interface type: <1=> i2c, <2=> spi
#define __GH30X_COMMUNICATION_INTERFACE__   (GH30X_COMMUNICATION_INTERFACE_I2C)

/// gh30x default irq pluse width 20us, if need config irq pluse width, search this macro.
#define __GH30X_IRQ_PLUSE_WIDTH_CONFIG__    (1)

/// platform delay us config
#define __PLATFORM_DELAY_US_CONFIG__        (0)

/// retry max cnt
#define __RETRY_MAX_CNT_CONFIG__            (3)

/// reinit max cnt
#define __RESET_REINIT_CNT_CONFIG__         (5)


/// fifo int timeout check 
#define __FIFO_INT_TIMEOUT_CHECK__          (0)

/// get rawdata buffer length, max 256 (use heap space)
#define __GET_RAWDATA_BUF_LEN__             (256)

/* ble */

/// if need use GOODIX app debug
#define __USE_GOODIX_APP__                  (0)

/// if need mutli pkg, = (__NEW_DATA_MULTI_PKG_NUM__ + 1), so set 1 that send with two rawdata cmd
#define __NEW_DATA_MULTI_PKG_NUM__          (4) // mean 5

/// ble pkg size max suport, should < (mtu - 3)
#define __BLE_PKG_SIZE_MAX__                (180)

/// ble mcu mode pkg buffer len
#define __BLE_MCU_PKG_BUFFER_MAX_LEN__      (3000)


/* uart */

/// uart connect to goodix dongle or production test tools 
#define __UART_WITH_GOODIX_TOOLS__          (0)


/* system test */

/// system test moudle support 
#define __SYSTEM_TEST_SUPPORT__             (0)

/// system test moudle cnt check noise, must <= 100
#define __SYSTEM_TEST_DATA_CNT_CONFIG__     (100)


/* dbg log lvl */

/// log debug lvl: <0=> off , <1=> normal info ,  <2=> with data info
#define __EXAMPLE_DEBUG_LOG_LVL__           (1)

/// log support len
#define __EXAMPLE_LOG_DEBUG_SUP_LEN__       (128)


#endif /* _GH3011_EXAMPLE_CONFIG_H_ */

/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
