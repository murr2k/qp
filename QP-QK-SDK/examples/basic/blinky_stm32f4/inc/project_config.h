/**
 * @file project_config.h
 * @brief Project Configuration for Blinky STM32F4 Example
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Configuration header for the Blinky example project.
 * Contains platform-specific settings and project parameters.
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include "qpc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
// PLATFORM CONFIGURATION
//============================================================================

// System clock configuration
#define BSP_TICKS_PER_SEC       1000U        // 1ms system tick
#define BSP_SYSTEM_CLOCK_HZ     168000000U   // 168 MHz for STM32F4

// QK kernel configuration
#define QK_PREEMPTION_PRIO      32U          // Maximum priority levels

//============================================================================
// ACTIVE OBJECT PRIORITIES
//============================================================================

enum ActiveObjectPriorities {
    AO_IDLE_PRIO = 0U,          // Reserved for idle (never used)
    AO_BLINKY_PRIO = 3U,        // Blinky Active Object priority
    MAX_AO_PRIO = QK_PREEMPTION_PRIO - 1U
};

//============================================================================
// EVENT POOL CONFIGURATION
//============================================================================

// Memory pools for events
#define SMALL_EVENT_POOL_SIZE   10U
#define MEDIUM_EVENT_POOL_SIZE  5U
#define LARGE_EVENT_POOL_SIZE   2U

//============================================================================
// BOARD SUPPORT PACKAGE CONFIGURATION
//============================================================================

// LED configuration (STM32F4 Discovery board)
#define LED_COUNT               4U
#define LED1_PIN                GPIO_PIN_12   // Green LED (LD4)
#define LED2_PIN                GPIO_PIN_13   // Orange LED (LD3)
#define LED3_PIN                GPIO_PIN_14   // Red LED (LD5)
#define LED4_PIN                GPIO_PIN_15   // Blue LED (LD6)
#define LED_GPIO_PORT           GPIOD
#define LED_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()

// Button configuration
#define BUTTON_PIN              GPIO_PIN_0
#define BUTTON_GPIO_PORT        GPIOA
#define BUTTON_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUTTON_EXTI_IRQn        EXTI0_IRQn

// UART configuration for QS trace output
#define QS_UART                 USART2
#define QS_UART_CLK_ENABLE()    __HAL_RCC_USART2_CLK_ENABLE()
#define QS_UART_GPIO_PORT       GPIOA
#define QS_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define QS_UART_TX_PIN          GPIO_PIN_2
#define QS_UART_RX_PIN          GPIO_PIN_3
#define QS_UART_AF              GPIO_AF7_USART2
#define QS_UART_BAUDRATE        115200U

//============================================================================
// QS SOFTWARE TRACING CONFIGURATION
//============================================================================

#ifdef Q_SPY
    // QS buffer sizes
    #define QS_TX_BUFFER_SIZE   2048U
    #define QS_RX_BUFFER_SIZE   256U
    #define QS_TSTAMP_SIZE      4U
    
    // Enable all QS trace groups for debugging
    #define QS_GLB_FILTER_INIT  QS_ALL_RECORDS
    
    // Custom trace records
    enum BlinkyQSUserRecords {
        QS_USER_00 = QS_USER,   // LED state changes
        QS_USER_01,             // Button events
        QS_USER_02,             // Timing information
        QS_USER_03,             // Performance data
        QS_USER_04              // Reset events
    };
    
#endif // Q_SPY

//============================================================================
// APPLICATION CONFIGURATION
//============================================================================

// Application version
#define APP_VERSION_MAJOR       1U
#define APP_VERSION_MINOR       0U
#define APP_VERSION_PATCH       0U

// Timing parameters
#define BLINK_FREQUENCY_HZ      1U              // 1 Hz blink rate
#define BLINK_PERIOD_MS         (1000U / BLINK_FREQUENCY_HZ / 2U)
#define BLINK_PERIOD_TICKS      (BSP_TICKS_PER_SEC * BLINK_PERIOD_MS / 1000U)

// Performance constraints
#define MAX_RTC_DURATION_MS     10U             // Maximum Run-to-Completion time
#define WATCHDOG_TIMEOUT_MS     5000U           // Watchdog timeout

//============================================================================
// MEMORY MANAGEMENT
//============================================================================

// Event structure sizes for pool configuration
typedef struct {
    QEvt super;
} SmallEvt;

typedef struct {
    QEvt super;
    uint32_t data;
} MediumEvt;

typedef struct {
    QEvt super;
    uint32_t data[4];
} LargeEvt;

// Memory pool storage declarations
extern QF_MPOOL_EL(SmallEvt) l_smlPoolSto[SMALL_EVENT_POOL_SIZE];
extern QF_MPOOL_EL(MediumEvt) l_medPoolSto[MEDIUM_EVENT_POOL_SIZE];
extern QF_MPOOL_EL(LargeEvt) l_lrgPoolSto[LARGE_EVENT_POOL_SIZE];

//============================================================================
// ASSERT AND ERROR HANDLING
//============================================================================

// QP assertion handler
void Q_onAssert(char const *module, int loc);

// BSP functions
void BSP_init(void);
void BSP_terminate(int16_t result);
void BSP_ledOn(uint8_t led);
void BSP_ledOff(uint8_t led);
void BSP_ledToggle(uint8_t led);
uint32_t BSP_getTime(void);
uint32_t BSP_getTimeUs(void);
void BSP_tickHook(void);

//============================================================================
// QS SOFTWARE TRACING INTERFACE
//============================================================================

#ifdef Q_SPY
    void QS_onStartup(void);
    void QS_onCleanup(void);
    void QS_onFlush(void);
    QSTimeCtr QS_onGetTime(void);
    void QS_onCommand(uint8_t cmdId, uint32_t param1, 
                      uint32_t param2, uint32_t param3);
#endif

#ifdef __cplusplus
}
#endif

#endif // PROJECT_CONFIG_H