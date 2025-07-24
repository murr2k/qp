/**
 * @file project_template.h
 * @brief STM32F4 QK Project Template Header
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Template for STM32F4 projects using QP/C with QK kernel
 * This file serves as a template for AI agents to generate
 * platform-specific project configurations.
 */

#ifndef PROJECT_TEMPLATE_H
#define PROJECT_TEMPLATE_H

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
#define BSP_TICKS_PER_SEC    1000U
#define BSP_SYSTEM_CLOCK_HZ  168000000U  // 168 MHz for STM32F4

// QK kernel configuration
#define QK_PREEMPTION_PRIO   64U  // Maximum priority levels

// Memory pools configuration
#define SMALL_EVENT_POOL_SIZE   16U
#define MEDIUM_EVENT_POOL_SIZE  8U  
#define LARGE_EVENT_POOL_SIZE   4U

//============================================================================
// ACTIVE OBJECT PRIORITIES (QK-specific)
//============================================================================
// Priority assignment following Rate Monotonic Analysis principles
// Higher number = higher priority in QK

enum {
    AO_IDLE_PRIO = 0U,        // Reserved for idle (never used)
    
    // Low priority - non-critical background tasks
    AO_LOGGER_PRIO = 1U,      // Data logging, file operations
    AO_COMM_PRIO = 2U,        // Communication protocols
    
    // Medium priority - application logic
    AO_SENSOR_PRIO = 3U,      // Sensor data processing
    AO_ACTUATOR_PRIO = 4U,    // Actuator control
    AO_CONTROLLER_PRIO = 5U,  // Main application controller
    
    // High priority - time-critical tasks
    AO_SAFETY_PRIO = 6U,      // Safety monitoring
    AO_TIMER_PRIO = 7U,       // Time-critical operations
    
    // Highest priority - interrupt-like tasks
    AO_CRITICAL_PRIO = 8U,    // Critical system tasks
    
    MAX_AO_PRIO = QK_PREEMPTION_PRIO - 1U
};

//============================================================================
// EVENT SIGNALS
//============================================================================

enum ProjectSignals {
    // Published signals (one-to-many)
    TICK_SIG = Q_USER_SIG,    // System tick event
    SENSOR_DATA_SIG,          // Sensor data available
    FAULT_SIG,                // System fault detected
    MODE_CHANGE_SIG,          // Operating mode change
    
    // Point-to-point signals
    START_SIG,                // Start operation
    STOP_SIG,                 // Stop operation
    RESET_SIG,                // Reset command
    CONFIG_SIG,               // Configuration change
    
    // Hardware interface signals
    GPIO_SIG,                 // GPIO state change
    TIMER_SIG,                // Timer event
    UART_RX_SIG,             // UART receive
    SPI_COMPLETE_SIG,        // SPI transaction complete
    
    // Add project-specific signals here
    // {{PROJECT_SIGNALS}}
    
    MAX_SIG                   // Keep last
};

//============================================================================
// EVENT STRUCTURES
//============================================================================

// Base event (no parameters)
typedef struct {
    QEvt super;
} BaseEvt;

// Sensor data event
typedef struct {
    QEvt super;
    uint16_t sensor_id;
    uint32_t timestamp;
    float value;
    uint8_t status;
} SensorDataEvt;

// Configuration event
typedef struct {
    QEvt super;
    uint16_t param_id;
    uint32_t value;
} ConfigEvt;

// GPIO event
typedef struct {
    QEvt super;
    uint16_t pin;
    uint8_t state;
} GpioEvt;

//============================================================================
// ACTIVE OBJECT DECLARATIONS
//============================================================================

// Forward declarations for Active Objects
// These will be generated based on project requirements

// {{ACTIVE_OBJECT_DECLARATIONS}}

//============================================================================
// BOARD SUPPORT PACKAGE (BSP) INTERFACE
//============================================================================

// BSP initialization and control
void BSP_init(void);
void BSP_terminate(int16_t result);
void BSP_ledOn(uint8_t led);
void BSP_ledOff(uint8_t led);
void BSP_ledToggle(uint8_t led);

// System timing
void BSP_tickHook(void);
uint32_t BSP_getTime(void);

// Hardware abstraction
void BSP_gpio_init(void);
void BSP_uart_init(void);
void BSP_spi_init(void);
void BSP_timer_init(void);

// Interrupt service routines
void BSP_systick_handler(void);

//============================================================================
// QS SOFTWARE TRACING CONFIGURATION
//============================================================================

#ifdef Q_SPY
    // QS buffer sizes
    #define QS_TX_BUFFER_SIZE   2048U
    #define QS_RX_BUFFER_SIZE   256U
    #define QS_TSTAMP_SIZE      4U
    
    // QS trace records
    enum QSUserRecords {
        QS_USER_00 = QS_USER,
        QS_SENSOR_DATA,           // Sensor data trace
        QS_GPIO_CHANGE,           // GPIO state change
        QS_TIMING_INFO,           // Timing measurements
        QS_ERROR_INFO,            // Error information
        // Add project-specific trace records
        // {{QS_USER_RECORDS}}
    };
    
    // QS initialization
    void QS_onStartup(void);
    void QS_onCleanup(void);
    void QS_onFlush(void);
    QSTimeCtr QS_onGetTime(void);
    
#endif // Q_SPY

//============================================================================
// ASSERT AND ERROR HANDLING
//============================================================================

// QP assertion handler
void Q_onAssert(char const *module, int loc);

// Error handling
typedef enum {
    ERROR_NONE = 0,
    ERROR_INIT_FAILED,
    ERROR_INVALID_PARAM,
    ERROR_TIMEOUT,
    ERROR_HARDWARE_FAULT,
    ERROR_MEMORY_FULL,
    ERROR_COMMUNICATION_LOST
} ErrorCode_t;

void BSP_error_handler(ErrorCode_t error, const char* file, int line);

//============================================================================
// MEMORY MANAGEMENT
//============================================================================

// Memory pool storage declarations
extern QF_MPOOL_EL(BaseEvt) l_smlPoolSto[SMALL_EVENT_POOL_SIZE];
extern QF_MPOOL_EL(SensorDataEvt) l_medPoolSto[MEDIUM_EVENT_POOL_SIZE];
extern QF_MPOOL_EL(ConfigEvt) l_lrgPoolSto[LARGE_EVENT_POOL_SIZE];

//============================================================================
// PROJECT-SPECIFIC CONFIGURATION
//============================================================================

// Application-specific constants
#define APP_VERSION_MAJOR    1U
#define APP_VERSION_MINOR    0U
#define APP_VERSION_PATCH    0U

// Hardware-specific settings
#define LED_COUNT           4U
#define SENSOR_COUNT        8U
#define UART_BAUD_RATE      115200U
#define SPI_CLOCK_RATE      1000000U

// Timing constraints (QK-specific)
#define MAX_RTC_DURATION_MS  10U    // Maximum Run-to-Completion time
#define WATCHDOG_TIMEOUT_MS  1000U  // Watchdog timeout

// {{PROJECT_SPECIFIC_CONFIG}}

#ifdef __cplusplus
}
#endif

#endif // PROJECT_TEMPLATE_H