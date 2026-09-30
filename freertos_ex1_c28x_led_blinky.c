/*
 * Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
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
 */

#include "driverlib.h"
#include "device.h"     // Device Headerfile and Examples Include File
#include "FreeRTOS.h"
#include "task.h"
//#include "semphr.h"
#include "event_groups.h"

#define STACK_SIZE  256U
//#define RED         0xDEADBEAF
//#define BLUE        0xBAADF00D

#define ADC_READY_BIT   (1U << 0)
#define UART_READY_BIT  (1U << 1)
static StaticTask_t adcTaskBuffer;
static StackType_t adcTaskStack[STACK_SIZE];

#pragma DATA_SECTION(adcTaskStack, ".freertosStaticStack")
#pragma DATA_ALIGN(adcTaskStack, portBYTE_ALIGNMENT)

static StaticTask_t uartTaskBuffer;
static StackType_t uartTaskStack[STACK_SIZE];

#pragma DATA_SECTION(uartTaskStack, ".freertosStaticStack")
#pragma DATA_ALIGN(uartTaskStack, portBYTE_ALIGNMENT)

static StaticTask_t systemTaskBuffer;
static StackType_t systemTaskStack[STACK_SIZE];

#pragma DATA_SECTION(systemTaskStack, ".freertosStaticStack")
#pragma DATA_ALIGN(systemTaskStack, portBYTE_ALIGNMENT)

//static StaticTask_t redTaskBuffer;
//static StackType_t  redTaskStack[STACK_SIZE];
//#pragma DATA_SECTION(redTaskStack,   ".freertosStaticStack")
//#pragma DATA_ALIGN ( redTaskStack , portBYTE_ALIGNMENT )
//
//static StaticTask_t blueTaskBuffer;
//static StackType_t  blueTaskStack[STACK_SIZE];
//#pragma DATA_SECTION(blueTaskStack,   ".freertosStaticStack")
//#pragma DATA_ALIGN ( blueTaskStack , portBYTE_ALIGNMENT )

static StaticTask_t idleTaskBuffer;
static StackType_t idleTaskStack[STACK_SIZE];
#pragma DATA_SECTION(idleTaskStack,   ".freertosStaticStack")
#pragma DATA_ALIGN ( idleTaskStack , portBYTE_ALIGNMENT )

//static SemaphoreHandle_t xSemaphore = NULL;
//static StaticSemaphore_t xSemaphoreBuffer;
static EventGroupHandle_t xSystemEventGroup = NULL;
static StaticEventGroup_t xSystemEventGroupBuffer;

static uint32_t adcReady = 0;
static uint32_t uartReady = 0;
static uint32_t systemReady = 0;

//-------------------------------------------------------------------------------------------------
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while (1)
        ;
}

//-------------------------------------------------------------------------------------------------
//static void blueLedToggle(void)
//{
//    static uint32_t counter = 0;
//
//    counter++;
//    GPIO_writePin(DEVICE_GPIO_PIN_LED1, counter & 1);
//}
//
////-------------------------------------------------------------------------------------------------
//static void redLedToggle(void)
//{
//    static uint32_t counter = 0;
//
//    counter++;
//    GPIO_writePin(DEVICE_GPIO_PIN_LED2, counter & 1);
//}

//-------------------------------------------------------------------------------------------------
//static void ledToggle(uint32_t led)
//{
//    if(RED == led)
//    {
//        redLedToggle();
//    }
//    else
//    if(BLUE == led)
//    {
//        blueLedToggle();
//    }
//}
//
// configCPUTimer - This function initializes the selected timer to the
// period specified by the "freq" and "period" variables. The "freq" is
// CPU frequency in Hz and the period in uSeconds. The timer is held in
// the stopped state after configuration.
//
void configCPUTimer(uint32_t cpuTimer, uint32_t period)
{
    uint32_t temp, freq = DEVICE_SYSCLK_FREQ;

    //
    // Initialize timer period:
    //
    temp = ((freq / 1000000) * period);
    CPUTimer_setPeriod(cpuTimer, temp);

    //
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CPUTimer_setPreScaler(cpuTimer, 0);

    //
    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    //
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(cpuTimer);

}

//-------------------------------------------------------------------------------------------------
//interrupt void timer1_ISR( void )
//{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//
//    xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
//
//    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//}

//-------------------------------------------------------------------------------------------------
//static void setupTimer1( void )
//{
//  //  Interrupt_register(INT_TIMER1, &timer1_ISR);
//
//    CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF);
//    CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);
//    CPUTimer_stopTimer(CPUTIMER1_BASE);
//    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
//
//    configCPUTimer(CPUTIMER1_BASE, 100000);
//
//    CPUTimer_enableInterrupt(CPUTIMER1_BASE);
//
//    Interrupt_enable(INT_TIMER1);
//    CPUTimer_startTimer(CPUTIMER1_BASE);
//}
void ADCTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        adcReady = 1;

        xEventGroupSetBits(xSystemEventGroup,
        ADC_READY_BIT);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void UARTTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));

        uartReady = 1;

        xEventGroupSetBits(xSystemEventGroup,
        UART_READY_BIT);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void SystemTask(void *pvParameters)
{
    EventBits_t events;

    for (;;)
    {
        events = xEventGroupWaitBits(xSystemEventGroup,
        ADC_READY_BIT | UART_READY_BIT,
                                     pdTRUE,
                                     pdTRUE,
                                     portMAX_DELAY);

        if ((events & (ADC_READY_BIT | UART_READY_BIT))
                == (ADC_READY_BIT | UART_READY_BIT))
        {
            systemReady = 1;

            GPIO_writePin(DEVICE_GPIO_PIN_LED1, 0);
            GPIO_writePin(DEVICE_GPIO_PIN_LED2, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

//-------------------------------------------------------------------------------------------------
//void LED_TaskRed(void * pvParameters)
//{
//    for(;;)
//    {
//        if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
//        {
//            ledToggle((uint32_t)pvParameters);
//        }
//    }
//}
//
////-------------------------------------------------------------------------------------------------
//void LED_TaskBlue(void * pvParameters)
//{
//    for(;;)
//    {
//        ledToggle((uint32_t)pvParameters);
//        vTaskDelay(250 / portTICK_PERIOD_MS);
//    }
//}

//-------------------------------------------------------------------------------------------------
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &idleTaskBuffer;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = STACK_SIZE;
}

//-------------------------------------------------------------------------------------------------
void main(void)
{
    //
    // Initializes device clock and peripherals
    //
    Device_init();

    //
    // Initializes PIE and clears PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    Device_initGPIO();
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);

    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 1);
    GPIO_writePin(DEVICE_GPIO_PIN_LED2, 1);

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initializes the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

//    xSemaphore = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );
    xSystemEventGroup = xEventGroupCreateStatic(&xSystemEventGroupBuffer);

//    setupTimer1();

    // Enable global Interrupts and higher priority real-time debug events:
    EINT;
    // Enable Global interrupt INTM
    ERTM;
    // Enable Global realtime interrupt DBGM

    // Create the task without using any dynamic memory allocation.
    xTaskCreateStatic(ADCTask, "ADC Task",
    STACK_SIZE,
                      NULL,
                      tskIDLE_PRIORITY + 2,
                      adcTaskStack, &adcTaskBuffer);

    xTaskCreateStatic(UARTTask, "UART Task",
    STACK_SIZE,
                      NULL,
                      tskIDLE_PRIORITY + 2,
                      uartTaskStack, &uartTaskBuffer);

    xTaskCreateStatic(SystemTask, "System Task",
    STACK_SIZE,
                      NULL,
                      tskIDLE_PRIORITY + 3,
                      systemTaskStack, &systemTaskBuffer);
//    xTaskCreateStatic(LED_TaskRed,          // Function that implements the task.
//                      "Red LED task",       // Text name for the task.
//                      STACK_SIZE,           // Number of indexes in the xStack array.
//                      ( void * ) RED,       // Parameter passed into the task.
//                      tskIDLE_PRIORITY + 2, // Priority at which the task is created.
//                      redTaskStack,         // Array to use as the task's stack.
//                      &redTaskBuffer );     // Variable to hold the task's data structure.
//
//    xTaskCreateStatic(LED_TaskBlue,         // Function that implements the task.
//                      "Blue LED task",      // Text name for the task.
//                      STACK_SIZE,           // Number of indexes in the xStack array.
//                      ( void * ) BLUE,      // Parameter passed into the task.
//                      tskIDLE_PRIORITY + 1, // Priority at which the task is created.
//                      blueTaskStack,        // Array to use as the task's stack.
//                      &blueTaskBuffer );    // Variable to hold the task's data structure.

    vTaskStartScheduler();
}
