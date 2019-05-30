/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/
#include "tinyprintf.h"

#define DPrintf	tfp_printf
/* Tick timer configuration. */
#define MHz                                         (1000000)
#define SYSCLK                                      ((unsigned long)100*MHz)
#define configTICK_TIMER_CHANNEL                    (timer_select_a)
#define configTICK_TIMER_PRESCALER                  (50)
#define configCPU_CLOCK_HZ                          ((unsigned long)(SYSCLK / configTICK_TIMER_PRESCALER))

/* These are used in porting code (Source\portable). */
#define configTICK_RATE_HZ                          ((TickType_t)1000)
/* To calculate the absolute maximum size of configTOTAL_HEAP_SIZE:
 * - Undefine the macro configSTATIC_STORAGE and compile project.
 * - Examine compiler output. Typical output will be as follows:
 *         ft32-elf-size --format=berkeley -x "project.elf"
           text    data   bss     dec     hex   filename
           0xTTTTT 0xDDDD 0xBBBB  dddddd  hhhhh AzureTest.elf
 * - Fill in values for "data" and "bss" in the definition of configSTATIC_STORAGE.
 *   FT900: #define configSTATIC_STORAGE ((size_t)(0xDDDD + 0xBBBB))
 * - Define configSTATIC_STORAGE and recompile project.
 * This will need to be redone should any changes be made to the global or
 * static variables in the project.
 */
//#define configSTATIC_STORAGE                        ((size_t)(0xDDDD + 0xBBBB))
#if !defined(configSTATIC_STORAGE)
#if defined(__FT900__)
#include <iot_config.h>
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
#define configTOTAL_HEAP_SIZE                       ((size_t)(48 * 1024))
#else
#define configTOTAL_HEAP_SIZE                       ((size_t)(46 * 1024))
#endif
#elif defined(__FT930__)
#define configTOTAL_HEAP_SIZE                       ((size_t)(26 * 1024))
#endif
#elif defined(__FT900__)
#define configTOTAL_HEAP_SIZE                       ((size_t)(0x10000 - 0x10 - configSTATIC_STORAGE))
#elif defined(__FT930__)
#define configTOTAL_HEAP_SIZE                       ((size_t)(0x8000 - 0x10 - configSTATIC_STORAGE))
#endif
#define configUSE_MALLOC_FAILED_HOOK                1

/* These are defined in FreeRTOS.h, mandatory. */
#define configMINIMAL_STACK_SIZE                    ((unsigned short)2*128)
#define configMAX_PRIORITIES                        5
#define configUSE_PREEMPTION                        0
#define configUSE_IDLE_HOOK                         0
#define configUSE_TICK_HOOK                         0
#define configUSE_16_BIT_TICKS                      0

/* These are defined in FreeRTOS.h, default 0 when !defined. */
#define configUSE_APPLICATION_TASK_TAG              0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS     0
#define configUSE_RECURSIVE_MUTEXES                 0
#define configUSE_MUTEXES                           1
#define configUSE_COUNTING_SEMAPHORES               0
#define configUSE_ALTERNATIVE_API                   0
#define configUSE_TIMERS                            1
#define configQUEUE_REGISTRY_SIZE                   0U
#define configCHECK_FOR_STACK_OVERFLOW              0       //0, 1, or 2
#define configUSE_QUEUE_SETS                        0
#define configUSE_NEWLIB_REENTRANT                  0
#define portTICK_TYPE_IS_ATOMIC                     0
#define portCRITICAL_NESTING_IN_TCB					0

/* These are defined in FreeRTOS.h, default 0 when !defined. For debug use. */
#define configGENERATE_RUN_TIME_STATS               0
#define configUSE_STATS_FORMATTING_FUNCTIONS        0
#define configUSE_TRACE_FACILITY                    0

 #define configSUPPORT_STATIC_ALLOCATION            0
 #define configSUPPORT_DYNAMIC_ALLOCATION           1

/* These are defined in FreeRTOS.h, default non-zero when !defined. */
#define configMAX_TASK_NAME_LEN                     (16)    //Default 16
#define configIDLE_SHOULD_YIELD                     0       //Default 1
#define configUSE_TIME_SLICING                      0       //Default 1
#define configUSE_TASK_NOTIFICATIONS                1       //Default 1
#define configENABLE_BACKWARD_COMPATIBILITY         0       //Default 1

/* These must be defined when configUSE_TIMERS==1. */
#if (configUSE_TIMERS == 1)
#define configTIMER_TASK_PRIORITY                   (configMAX_PRIORITIES)
#define configTIMER_QUEUE_LENGTH                    (3)
#define configTIMER_TASK_STACK_DEPTH                (256)
#endif

#ifdef DEBUG
#define configASSERT(x)                                                 \
    {                                                                   \
        if((int)(x) == 0) {                                             \
            DPrintf("%s() #%x\r\n", __FUNCTION__, __LINE__);            \
        }                                                               \
    }
#else
//#define configASSERT(x...) (void)(x)
#endif


/* Set the following definitions to 1 to include the API function, or zero to exclude the API function. */
/* Mandatory. */
#define INCLUDE_vTaskPrioritySet                    0
#define INCLUDE_uxTaskPriorityGet                   0
#define INCLUDE_vTaskDelete                         0
#define INCLUDE_vTaskSuspend                        0
#define INCLUDE_vTaskDelayUntil                     0
#define INCLUDE_vTaskDelay                          1

/* Optional, default to 0 when !defined. */
#define INCLUDE_xTaskGetIdleTaskHandle              0
#define INCLUDE_xTimerGetTimerDaemonTaskHandle      0
#define INCLUDE_xQueueGetMutexHolder                0
#define INCLUDE_xSemaphoreGetMutexHolder            0
#define INCLUDE_pcTaskGetTaskName                   0
#define INCLUDE_uxTaskGetStackHighWaterMark         0
#define INCLUDE_eTaskGetState                       0
#define INCLUDE_xTaskResumeFromISR                  0
#define INCLUDE_xEventGroupSetBitFromISR            0
#define INCLUDE_xTimerPendFunctionCall              0
#define INCLUDE_xTaskGetSchedulerState              0
#define INCLUDE_xTaskGetCurrentTaskHandle           0
#define INCLUDE_vTaskCleanUpResources               0

/* Trace. */
//#define traceSTART()                                /* Available only for FreeRTOS+Trace? */
//#define traceEND()                                  /* Available only for FreeRTOS+Trace? */
//#define traceTASK_PRIORITY_INHERIT(pxTCBOfMutexHolder, uxInheritedPriority)     /* TODO */
//#define traceTASK_PRIORITY_DISINHERIT(pxTCBOfMutexHolder, uxOriginalPriority)   /* TODO */
//#define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue)     /* TODO */
//#define traceBLOCKING_ON_QUEUE_SEND(pxQueue)        /* TODO */

//#define traceTASK_SWITCHED_IN()                     portTASK_SWITCHED('I')
//#define traceTASK_SWITCHED_OUT()                    portTASK_SWITCHED('O')
//#define traceTASK_CREATE                            portTASK_CREATE
//#define traceTASK_CREATE_FAILED                     portTASK_CREATE_FAILED
//#define traceTASK_DELAY                             portTASK_DELAY

#endif /* FREERTOS_CONFIG_H */
