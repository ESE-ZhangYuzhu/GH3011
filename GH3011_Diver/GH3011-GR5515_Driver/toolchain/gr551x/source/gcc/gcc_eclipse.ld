/* Linker script to configure memory regions. */

#include "custom_config.h"


#define ROM_RTN_RAM_SIZE        0x4000
#define RAM_START_ADDR      0x30000000
#define RAM_SIZE            0x00040000
#define RAM_END_ADDR        (RAM_START_ADDR+RAM_SIZE)

#define FLASH_START_ADDR    0x01000000
#define FLASH_BOOTINFO_SIZE 0x00002000
#define FLASH_SIZE          0x00800000
#define FERP_SIZE           0x8000    //32K
#define EXFLASH_DRIVER_CODE_SIZE  0x8000    // size of exflash driver reserved



#if (APP_CODE_RUN_ADDR == APP_CODE_LOAD_ADDR && \
		APP_CODE_RUN_ADDR >= FLASH_START_ADDR && \
		APP_CODE_RUN_ADDR < FLASH_START_ADDR + FLASH_SIZE)
    #define XIP_MODE    
#endif

#ifdef XIP_MODE
#define MAX_APP_CODE_SIZE (FLASH_SIZE-FLASH_BOOTINFO_SIZE)
#define MAX_APP_USED_RAM_SIZE (RAM_SIZE-ROM_RTN_RAM_SIZE)
#else
#define MAX_APP_CODE_SIZE (RAM_START_ADDR+RAM_SIZE-APP_CODE_RUN_ADDR-CSTACK_HEAP_SIZE)
#define MAX_APP_USED_RAM_SIZE (APP_CODE_RUN_ADDR-RAM_START_ADDR-ROM_RTN_RAM_SIZE)
#endif



MEMORY
{
  FLASH (rx)  : ORIGIN = APP_CODE_RUN_ADDR, LENGTH = MAX_APP_CODE_SIZE
  RAM   (rwx) : ORIGIN = RAM_START_ADDR+ROM_RTN_RAM_SIZE, LENGTH = MAX_APP_USED_RAM_SIZE
}

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 */
ENTRY(Reset_Handler)

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		__Vectors_End = .;
		__Vectors_Size = __Vectors_End - __Vectors;
		__end__ = .;
        
		*(EXCLUDE_FILE(./DRIVERS/gr55xx_hal_xqspi.o ./DRIVERS/gr55xx_hal_exflash.o ./DRIVERS/gr55xx_ll_xqspi.o ../lib/rwip_sleep.o ../lib/rf_gdx.o) .text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
 		*(SORT(.dtors.*))
 		*(.dtors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
	} > FLASH

	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;


	__etext = .;

	.data : AT (__etext)
	{
		__data_start__ = .;
		*(vtable)
		*(.data*)
        *(RAM_CODE)
        ./DRIVERS/gr55xx_hal_xqspi.o(.text*)
        ./DRIVERS/gr55xx_hal_exflash.o(.text*)
        ./DRIVERS/gr55xx_ll_xqspi.o(.text*)
        ../lib/rwip_sleep.o(.text*)
        ../lib/rf_gdx.o(.text*)
		
        . = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);


		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;

	} > RAM

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.heap (COPY):
	{
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		KEEP(*(.heap*))
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY):
	{
		KEEP(*(.stack*))
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
