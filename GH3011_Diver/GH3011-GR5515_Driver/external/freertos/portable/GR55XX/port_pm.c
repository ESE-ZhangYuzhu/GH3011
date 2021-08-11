
/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include <stdio.h>
#include "gr55xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gr55xx_hal.h"
#include "gr55xx_sys.h"
#include "app_timer.h"
#include "semphr.h"

extern void before_sleep_callback(void);
static app_timer_id_t s_rtos_timer_hdl = 0;

typedef enum
{
   AON_TIMER_READY = 0,
   AON_TIMER_RUN_FINISH,    
}RTOS_TIMER_RUN_STATE_t;

static volatile RTOS_TIMER_RUN_STATE_t s_aon_timer_build_finish = AON_TIMER_READY;

TINY_RAM_SECTION uint32_t vPortLocker(void)
{
   uint32_t ret_pri = __get_PRIMASK();
   __set_PRIMASK(1); 
   return ret_pri;
}

TINY_RAM_SECTION void vPortUnLocker(uint32_t set_pri)
{
   __set_PRIMASK(set_pri); 
}

TINY_RAM_SECTION __STATIC_INLINE void SysTickStop(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

TINY_RAM_SECTION __STATIC_INLINE void SysTickStart(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

TINY_RAM_SECTION void SysTickReload(uint32_t CyclesPerTick)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD  = (uint32_t)(CyclesPerTick - 1UL);                 /* set reload register */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk \
                  | SysTick_CTRL_CLKSOURCE_Msk \
                  | SysTick_CTRL_TICKINT_Msk;
}

TINY_RAM_SECTION static void aon_timer_build_handler(void* p_ctx)
{
     s_aon_timer_build_finish = AON_TIMER_RUN_FINISH;
}

TINY_RAM_SECTION void wfe_func(void)
{
     pwr_mgmt_wfe_sleep();
}

TINY_RAM_SECTION static void pwr_mgmt_enter_sleep_with_cond(int pwr_mgmt_expected_time)
{  
      static int s_total_sleep_ticks = 0;
      s_total_sleep_ticks = pwr_mgmt_expected_time;


      /* Open app timer only when there is a specific next task to execute. */
      if (s_total_sleep_ticks > 0)
      {
         /* Only create the timer once. */
         static uint8_t timer_init_flag = 0;
         if ( !timer_init_flag )
         {
            app_timer_create( &s_rtos_timer_hdl, ATIMER_ONE_SHOT, aon_timer_build_handler);
            timer_init_flag = 0x1;
         }

         /* Set this flag to ready. */
         s_aon_timer_build_finish = AON_TIMER_READY;

         /* To start the app timer. */
         app_timer_start(s_rtos_timer_hdl, s_total_sleep_ticks , NULL);
      }
    
      if ( DEVICE_BUSY == pwr_mgmt_dev_suspend() )
      {
          s_aon_timer_build_finish = AON_TIMER_READY;
          app_timer_stop(s_rtos_timer_hdl);
          wfe_func();
          return ;
      }
      
      /* To disbale global IRQ. */
      uint32_t pwr_locker = vPortLocker();
             
      /* Check whether the ble core has been powered on again. */             
      if ( pwr_mgmt_baseband_state_get() != PMR_MGMT_SLEEP_MODE )
      {
         s_aon_timer_build_finish = AON_TIMER_READY;
         app_timer_stop(s_rtos_timer_hdl);
         vPortUnLocker(pwr_locker);
         wfe_func();
         return ;
      }

      /* Before the lock, 
         there was an interrupt context that changed the sleep mode and needed to take effect immediately. */
      if ( PMR_MGMT_SLEEP_MODE != pwr_mgmt_mode_get() )
      {
          s_aon_timer_build_finish = AON_TIMER_READY;
          app_timer_stop(s_rtos_timer_hdl);
          vPortUnLocker(pwr_locker);
          return ;
      }
    
      /* Save the context of RTOS. */
      pwr_mgmt_save_context();
      
      /* Judge the current startup mode mark. */
      if ( pwr_mgmt_get_wakeup_flag() == COLD_BOOT )
      {  
            /* To stop the systick. */
            SysTickStop();
          
            /* Shutdown all system power and wait some event to wake up. */
            if ( PMR_MGMT_IDLE_MODE == pwr_mgmt_shutdown() )
            { 
                s_aon_timer_build_finish = AON_TIMER_READY;
  
                /* To stop the app timer. */
                app_timer_stop(s_rtos_timer_hdl);
               
                /* To start the systick. */
                SysTickStart();
                
                 /* Unlock and Wait Timer Interrupt Since the Safe Check Failure. */ 
                vPortUnLocker(pwr_locker);
                 
                wfe_func();  
                 
                return ;
            }

            s_aon_timer_build_finish = AON_TIMER_READY;
  
            /* To stop the app timer. */
            app_timer_stop(s_rtos_timer_hdl);
           
            /* To start the systick. */
            SysTickStart();
            
            /* To enable global IRQ. */
            vPortUnLocker(pwr_locker);

      }
      else //wekeup from deep sleep mode
      {   
          /* clear wakeup mark ,parpare next time enter-sleep action. */
          pwr_mgmt_set_wakeup_flag(COLD_BOOT);
          
          /* To disbale global IRQ. */
          uint32_t _local_lock = vPortLocker();
          
          /* Restart SysTick module. */
          SysTickReload( configCPU_CLOCK_HZ / configTICK_RATE_HZ );
  
          /* Check whether it is another wake-up source. */
          if ( AON_TIMER_RUN_FINISH == s_aon_timer_build_finish )
          {
			  s_aon_timer_build_finish = AON_TIMER_READY;
						   
              /* Compensation for system counter of RTOS. */
              vTaskStepTick( s_total_sleep_ticks - 1 );
          }
          else
          {
              if (s_total_sleep_ticks > 0)
              {
                 /* To get the elapsed time from app-timer. */
                 int ticks =  app_timer_stop_and_ret(s_rtos_timer_hdl);
                 if ( ticks > 1000 )  
                    ticks /= 1000;
                 else
                    ticks = 1;
                
                 /* Compensation for system counter of RTOS. */
                 vTaskStepTick( ticks );
              }
          }
          
          /* To enable global IRQ. */
          vPortUnLocker(pwr_locker);
          
          extern void warm_boot_second(void);
          warm_boot_second();
      }
      return ;
}

/**
 *****************************************************************************************
 * @brief vPortEnterDeepSleep
 *
 * @param[in] xExpectedIdleTime: next task resume time to work
 *
 * @return void
 *****************************************************************************************
 */
TINY_RAM_SECTION void vPortEnterDeepSleep( TickType_t xExpectedIdleTime )
{
    //todo:
    /* Check ble event-list, to enable IRQ if any-bits in list has pending. */
    pwr_mgmt_check_ble_event();
	 
    if ( xExpectedIdleTime < 5 )
    {
       wfe_func();
       return ;        
    }
    
	if ( PMR_MGMT_ACTIVE_MODE == pwr_mgmt_mode_get() )
	{
       wfe_func();
	   return ;
	}
		
    uint32_t pwr_locker = vPortLocker();
    
    /* Set ble core enter sleep ASAP. */
    pwr_mgmt_mode_t sleep_mode = pwr_mgmt_baseband_state_get();
    
    /* Disable locker. */
    vPortUnLocker(pwr_locker);
    
    switch (sleep_mode)
    {
        case PMR_MGMT_ACTIVE_MODE:
        case PMR_MGMT_IDLE_MODE:
             /* ARM enter idle mode. */
             wfe_func();
             break;
        
        case PMR_MGMT_SLEEP_MODE:
             /* System enter deep mode. */
             pwr_mgmt_enter_sleep_with_cond(xExpectedIdleTime);
             break;
    }
    return ;
}
