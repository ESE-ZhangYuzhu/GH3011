/**
 ****************************************************************************************
 *
 * @file scatter_config.h
 *
 * @brief Common scatter file definition file.
 *
 *
 ****************************************************************************************
 */

#ifndef __SCATTER_CONFIG_H__
#define __SCATTER_CONFIG_H__

#include "custom_config.h"

/*****************************************************************
 * if CSTACK_HEAP_SIZE is not defined in custom_config.h, 
 * keep default setting to 32KB
 */
#ifndef CSTACK_HEAP_SIZE
    #define CSTACK_HEAP_SIZE        0x8000
#endif

#define FLASH_START_ADDR          0x01000000
#define FLASH_SIZE                0x00800000

/* size of ROM reserved RAM in retention cell */
#ifndef ROM_RTN_RAM_SIZE
    #define ROM_RTN_RAM_SIZE        0x4020
#endif

#define RAM_ALIAS

/*****************************************************************
 * Warning: User App developer never change the six macros below
 */
#ifdef RAM_ALIAS
#define RAM_START_ADDR      0x00800000
#else
#define RAM_START_ADDR      0x30000000
#endif

#define RAM_SIZE            0x00040000
#define RAM_END_ADDR        (RAM_START_ADDR+RAM_SIZE)


#define FERP_SIZE               0x8000    //32K
#define CRITICAL_CODE_MAX_SIZE  0x10000    // maximum size of critical code reserved

#if (APP_CODE_RUN_ADDR == APP_CODE_LOAD_ADDR && \
        APP_CODE_RUN_ADDR >= FLASH_START_ADDR && \
        APP_CODE_RUN_ADDR < FLASH_START_ADDR + FLASH_SIZE)
    #define XIP_MODE    
#endif
/****************************************************************/

/* ************************************************************************
 * developer must define CFG_MAX_CONNECTIONS in custom_config.h .
 * Max value for GR551X: 10 which must be same with CFG_CON
 * in ROM's configs.opt
 */
#ifndef CFG_MAX_CONNECTIONS
    #error "CFG_MAX_CONNECTIONS is not defined in app's custom_config.h ."
#endif

#if (CFG_MAX_CONNECTIONS <= 10)
    #define USER_MAX_CONNECTIONS CFG_MAX_CONNECTIONS
#else
    #define USER_MAX_CONNECTIONS (10)
#endif

#ifndef CFG_MAX_BOND_DEV_NUM
    #error "CFG_MAX_BOND_DEV_NUM is not defined in app's custom_config.h ."
#endif

#if (CFG_MAX_BOND_DEV_NUM <= 10)
    #define USER_MAX_BOND_DEV_NUM CFG_MAX_BOND_DEV_NUM
#else
    #define USER_MAX_BOND_DEV_NUM (10)
#endif

#ifndef CFG_MAX_PRF_NB
    #error "CFG_MAX_PRF_NB is not defined in app's custom_config.h ."
#endif

#if (CFG_MAX_PRF_NB <= 64)
    #define USER_MAX_PRF_NB CFG_MAX_PRF_NB
#else
    #define USER_MAX_PRF_NB (64)
#endif

#define ENV_HEAP_SIZE       (1464 + USER_MAX_CONNECTIONS * 788)
/* The size of heap for ATT database depends on the number of attributes in
 * profiles. The value can be tuned based on supported profiles. */
#if (CFG_MESH_SUPPORT == 1)
#include "mesh_stack_config.h"
#define ATT_DB_HEAP_SIZE    (1000 + MESH_HEAP_SIZE_ADD)
#else
#define ATT_DB_HEAP_SIZE    (3072 + 12)
#endif
#define KE_MSG_HEAP_SIZE    (13420 + USER_MAX_CONNECTIONS * 520)
/* The size of non-retention heap is customized. This heap will used by BLE
 * stack only when other three heaps are full. */
#define NON_RET_HEAP_SIZE   (328 * 2 + 12)

#define PRF_BUF_SIZE 	(92*USER_MAX_PRF_NB)
#define BOND_BUF_SIZE 	(8*USER_MAX_BOND_DEV_NUM)
#define CONN_BUF_SIZE 	(372*USER_MAX_CONNECTIONS)


/**************************************************************************/
/* sections on retention RAM cells */
#ifdef CFG_FERP
    #define STACK_END_ADDR         (RAM_END_ADDR-FERP_SIZE)
#else
    #define STACK_END_ADDR         (RAM_END_ADDR)
#endif

#define USE_TINY_RAM_SPACE

#define TINY_RAM_SPACE_GAP    0x200
#define TINY_RAM_SPACE_START (RAM_START_ADDR + 0x2000)
#define TINY_RAM_SPACE_SIZE  (0x2000 - TINY_RAM_SPACE_GAP)

#define FPB_SECTION_START    0x30004000
#define FPB_SECTION_SIZE     0x20


#endif // __SCATTER_CONFIG_H__

