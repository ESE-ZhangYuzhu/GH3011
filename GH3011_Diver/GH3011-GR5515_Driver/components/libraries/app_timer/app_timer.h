#ifndef __APP_TIMER_H
#define __APP_TIMER_H

#include "gr55xx_sys.h"
#include <stdint.h>
#include <stdbool.h>


/**@brief The timer node trigger function. */
typedef void (*app_timer_fun_t)(void* p_ctx);

/**@brief App timer global variable. */
typedef struct
{
    uint8_t                  timer_node_used;
    uint8_t                  timer_node_status;
    uint8_t                  next_trigger_mode;
    uint32_t                 original_delay;
    uint32_t                 next_trigger_time;
    void*                    next_trigger_callback_var;
    app_timer_fun_t          next_trigger_callback;
}app_timer_t;

/**@brief The timer node id. */
typedef app_timer_t* app_timer_id_t; 


/**@brief App timer trigger types. */
typedef enum
{
   ATIMER_ONE_SHOT = 0x0,
   ATIMER_REPEAT 
} app_timer_type_t;


/**
 *****************************************************************************************
 * @brief  create a software timer.
 *
 * @param[in] p_timer_id:  the id of timer node
 * @param[in] mode:        timer trigger mode.
 * @param[in] callback:    Pointer to timer expire callback function
 *
 * @return the error code of this funciton

 * @note   After the function is executed, a new soft timer is added to the list and waits 
 *         for execution. At this point, the user can operate the app_timer_start/app_timer_stop 
 *         function to pause and restore the timer. These two functions do not delete the 
 *         timer node. If the user wants to delete a soft timer node completely, user can call 
 *         the app_timer_remove function, in which the create/remove function needs to pass in 
 *         the pointer, because these functions update the value of the pointer.
 * @note   It should be noted that the minimum delay time of this function is 10 ms. The purpose 
 *         is to enhance the execution delay of soft timer and improve the real-time performance 
 *         of soft timer.
 * @note   If the return value of this function is SDK_ERR_LIST_FULL, please find the macro 
 *         TIMER_NODE_CNT in app_timer.h and modify the value.
 *****************************************************************************************
 */
sdk_err_t app_timer_create(app_timer_id_t *p_timer_id, app_timer_type_t mode, app_timer_fun_t callback);

/**
 *****************************************************************************************
 * @brief  Gets the current app timer's switching status
 * @return state 0 means stop 1 means open
 *****************************************************************************************
 */
uint8_t app_timer_get_status(void);

/**
 *****************************************************************************************
 * @brief  To stop a existed timer in node list
 * @param[in] p_timer_id: the id of timer node
 *****************************************************************************************
 */
void app_timer_stop(app_timer_id_t p_timer_id);

/**
 *****************************************************************************************
 * @brief To start a existed timer in node list with old parameters
 * @param[in] app_timer_id_t: the id of timer node
 * @param[in] delay : the delay value of timer node, note this value should not
 *                       exceed 4000 seconds. Unit (ms).
 * @param[in] p_ctx : the pointer of context
 *****************************************************************************************
 */
sdk_err_t app_timer_start(app_timer_id_t p_timer_id, uint32_t delay, void *p_ctx);

/**
 *****************************************************************************************
 * @brief Delete a timer node from list .
 * @param[in] p_timer_id: the timer of id will be removed from timer list, and parameter 
 *                        will be set to NULL
 *****************************************************************************************
 */
sdk_err_t app_timer_delete(app_timer_id_t *p_timer_id);

/**
 *****************************************************************************************
 * @brief  Stop the currently running timer and return the running time
 *
 * @param[in] p_timer_handle:  Pointer to the timer handle
 *
 * @return  The time that the current timer has run
 *****************************************************************************************
 */
uint32_t app_timer_stop_and_ret(app_timer_id_t p_timer_id);

#endif
