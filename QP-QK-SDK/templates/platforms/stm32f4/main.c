/**
 * @file main.c
 * @brief STM32F4 QK Main Application Template
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Template main.c for STM32F4 projects using QP/C with QK kernel
 * This file serves as a template for AI agents to generate
 * platform-specific main application files.
 */

#include "project_template.h"

Q_DEFINE_THIS_FILE

//============================================================================
// LOCAL STORAGE FOR ACTIVE OBJECTS
//============================================================================

// {{ACTIVE_OBJECT_STORAGE_DECLARATIONS}}

//============================================================================
// EVENT POOL STORAGE
//============================================================================

// Small event pool (events with no parameters)
QF_MPOOL_EL(BaseEvt) l_smlPoolSto[SMALL_EVENT_POOL_SIZE];

// Medium event pool (sensor data, GPIO events)
QF_MPOOL_EL(SensorDataEvt) l_medPoolSto[MEDIUM_EVENT_POOL_SIZE];

// Large event pool (configuration events)
QF_MPOOL_EL(ConfigEvt) l_lrgPoolSto[LARGE_EVENT_POOL_SIZE];

//============================================================================
// PUBLISH-SUBSCRIBE STORAGE
//============================================================================

// Subscriber list storage for published events
static QSubscrList l_subscrSto[MAX_SIG];

//============================================================================
// QS SOFTWARE TRACING SETUP
//============================================================================

#ifdef Q_SPY

// QS transmit and receive buffers
static uint8_t l_qsTxBuf[QS_TX_BUFFER_SIZE];
static uint8_t l_qsRxBuf[QS_RX_BUFFER_SIZE];

// QS time stamp size
#if (QS_TSTAMP_SIZE == 1U)
    static uint8_t l_qs_tstamp;
#elif (QS_TSTAMP_SIZE == 2U)
    static uint16_t l_qs_tstamp;
#elif (QS_TSTAMP_SIZE == 4U)
    static uint32_t l_qs_tstamp;
#else
    #error "QS timestamp size not supported"
#endif

#endif // Q_SPY

//============================================================================
// MAIN FUNCTION
//============================================================================

int main(void) {
    
    // Initialize QF framework
    QF_init();
    
    // Initialize Board Support Package
    BSP_init();
    
    // Initialize event pools
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF_poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));
    QF_poolInit(l_lrgPoolSto, sizeof(l_lrgPoolSto), sizeof(l_lrgPoolSto[0]));
    
    // Initialize publish-subscribe mechanism
    QActive_psInit(l_subscrSto, Q_DIM(l_subscrSto));
    
#ifdef Q_SPY
    // Initialize QS software tracing
    if (!QS_INIT(l_qsTxBuf, sizeof(l_qsTxBuf), 
                 l_qsRxBuf, sizeof(l_qsRxBuf))) {
        Q_ERROR();
    }
    
    // Setup QS filters
    QS_GLB_FILTER(QS_ALL_RECORDS);   // Enable all QS records
    QS_GLB_FILTER(-QS_QF_TICK);      // Disable tick records (too frequent)
    
    // Enable local filters for specific Active Objects
    // QS_LOC_FILTER(QS_AO_OBJ, AO_MyActiveObject);
    
    // User-defined trace records
    QS_USR_DICTIONARY(QS_SENSOR_DATA);
    QS_USR_DICTIONARY(QS_GPIO_CHANGE);
    QS_USR_DICTIONARY(QS_TIMING_INFO);
    QS_USR_DICTIONARY(QS_ERROR_INFO);
    
    // Signal dictionary for readable trace output
    QS_SIG_DICTIONARY(TICK_SIG, (void *)0);
    QS_SIG_DICTIONARY(SENSOR_DATA_SIG, (void *)0);
    QS_SIG_DICTIONARY(FAULT_SIG, (void *)0);
    QS_SIG_DICTIONARY(MODE_CHANGE_SIG, (void *)0);
    QS_SIG_DICTIONARY(START_SIG, (void *)0);
    QS_SIG_DICTIONARY(STOP_SIG, (void *)0);
    QS_SIG_DICTIONARY(RESET_SIG, (void *)0);
    QS_SIG_DICTIONARY(CONFIG_SIG, (void *)0);
    QS_SIG_DICTIONARY(GPIO_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMER_SIG, (void *)0);
    QS_SIG_DICTIONARY(UART_RX_SIG, (void *)0);
    QS_SIG_DICTIONARY(SPI_COMPLETE_SIG, (void *)0);
    
    // {{QS_SIGNAL_DICTIONARY_ENTRIES}}
    
#endif // Q_SPY
    
    // Start Active Objects in priority order (lowest to highest)
    // This ensures proper initialization sequence
    
    // {{ACTIVE_OBJECT_START_SEQUENCE}}
    
    // Example Active Object start (template):
    /*
    static QEvt const *myAO_queueSto[10];
    static StackType_t myAO_stackSto[256];
    
    MyAO_ctor();  // Constructor
    QACTIVE_START(AO_MyAO,
                  AO_MYAO_PRIO,                    // Priority
                  myAO_queueSto, Q_DIM(myAO_queueSto), // Event queue
                  myAO_stackSto, sizeof(myAO_stackSto), // Stack (QXK only)
                  (void *)0);                      // Initialization parameter
    */
    
    // Transfer control to QK kernel
    return QF_run();
}

//============================================================================
// QF CALLBACKS
//============================================================================

void QF_onStartup(void) {
    // Enable interrupts that are used by the application
    
    // Configure system tick for QF
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
    
    // Set interrupt priorities for QK kernel
    // SysTick has the lowest priority among kernel-aware interrupts
    NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1U);
    
    // Configure other interrupt priorities
    // Example: UART interrupt
    // NVIC_SetPriority(USART2_IRQn, QF_AWARE_ISR_CMSIS_PRI + 2U);
    // NVIC_EnableIRQ(USART2_IRQn);
    
    // {{INTERRUPT_CONFIGURATION}}
}

void QF_onCleanup(void) {
    // Cleanup resources before termination
    BSP_terminate(0);
}

void QK_onIdle(void) {
    // Called when no Active Objects are ready to run
    // This is an ideal place for power management
    
    // Toggle heartbeat LED to show system is alive
    static uint32_t idle_counter = 0;
    if ((++idle_counter % 10000U) == 0U) {
        BSP_ledToggle(0);  // Heartbeat LED
    }
    
    // Put CPU into sleep mode (optional)
    // __WFI();  // Wait For Interrupt
    
    // QS trace output during idle time
#ifdef Q_SPY
    QS_rxParse();  // Parse QS-RX input
    QS_doOutput(); // Perform QS-TX output
#endif
}

//============================================================================
// QK KERNEL HOOKS
//============================================================================

void QK_onContextSw(QActive *prev, QActive *next) {
    // Called on every context switch in QK
    // Useful for profiling and debugging
    
#ifdef Q_SPY
    QS_BEGIN_ID(QS_SCHED_PREEMPT, 0U)
        QS_TIME_();                           // Timestamp
        QS_2U8_((uint8_t)prev->prio,          // Previous AO priority
                (uint8_t)next->prio);         // Next AO priority
    QS_END_()
#endif
    
    // Performance monitoring
    // Can be used to measure context switch overhead
}

//============================================================================
// ASSERTION AND ERROR HANDLING
//============================================================================

void Q_onAssert(char const *module, int loc) {
    // Called when a QP assertion fails
    // This is a serious error that should not occur in production
    
    // Disable all interrupts
    __disable_irq();
    
    // Turn on error LED
    BSP_ledOn(3);  // Error LED (red)
    
#ifdef Q_SPY
    // Log assertion failure
    QS_ASSERTION(module, loc, 10000U);
#endif
    
    // In debug mode, break into debugger
#ifdef DEBUG
    __BKPT(0);
#endif
    
    // In production, reset the system
    NVIC_SystemReset();
}

//============================================================================
// SYSTEM TICK INTERRUPT HANDLER
//============================================================================

void SysTick_Handler(void) {
    // Kernel-aware interrupt handling for QK
    QK_ISR_ENTRY();   // Inform QK kernel about ISR entry
    
    // Publish system tick event to all subscribers
    static QEvt const tick_evt = QEVT_INITIALIZER(TICK_SIG);
    QACTIVE_PUBLISH(&tick_evt, &l_SysTick_Handler);
    
    // Process QF time events
    QTIMEEVT_TICK_X(0U, &l_SysTick_Handler);
    
    // Call BSP tick hook for application-specific processing
    BSP_tickHook();
    
    QK_ISR_EXIT();    // Inform QK kernel about ISR exit
}

//============================================================================
// MEMORY MANAGEMENT CALLBACKS
//============================================================================

void QF_onClockTick(void) {
    // Called from system tick
    // Can be used for additional time-based processing
}

//============================================================================
// QS SOFTWARE TRACING CALLBACKS
//============================================================================

#ifdef Q_SPY

void QS_onStartup(void) {
    // Initialize hardware for QS output (typically UART)
    // This should be implemented in BSP
}

void QS_onCleanup(void) {
    // Cleanup QS resources
}

void QS_onFlush(void) {
    // Flush QS transmit buffer
    // Implementation depends on output method (UART, USB, etc.)
}

QSTimeCtr QS_onGetTime(void) {
    // Return current timestamp for QS
    return (QSTimeCtr)BSP_getTime();
}

void QS_onCommand(uint8_t cmdId, uint32_t param1, 
                  uint32_t param2, uint32_t param3) {
    // Handle commands received via QS-RX
    // Can be used for runtime system control
    switch (cmdId) {
        case 0U: {
            // Command 0: Toggle LED
            BSP_ledToggle((uint8_t)param1);
            break;
        }
        case 1U: {
            // Command 1: Reset system
            NVIC_SystemReset();
            break;
        }
        // Add more commands as needed
        // {{QS_COMMAND_HANDLERS}}
        default: {
            break;
        }
    }
}

#endif // Q_SPY

//============================================================================
// PROJECT-SPECIFIC INITIALIZATION
//============================================================================

// Add project-specific initialization functions here
// {{PROJECT_SPECIFIC_FUNCTIONS}}