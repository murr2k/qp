/**
 * @file main.c
 * @brief Main Application for Blinky STM32F4 Example
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Main application file for the Blinky example using QP/QK framework.
 * Demonstrates proper QK initialization and Active Object startup.
 */

#include "project_config.h"
#include "blinky.h"

Q_DEFINE_THIS_FILE

//============================================================================
// LOCAL STORAGE FOR ACTIVE OBJECTS
//============================================================================

// Event queue storage for Blinky Active Object
static QEvt const *l_blinkyQueueSto[BLINKY_QUEUE_LEN];

//============================================================================
// EVENT POOL STORAGE
//============================================================================

// Small event pool (basic events with no parameters)
QF_MPOOL_EL(SmallEvt) l_smlPoolSto[SMALL_EVENT_POOL_SIZE];

// Medium event pool (events with small data)
QF_MPOOL_EL(MediumEvt) l_medPoolSto[MEDIUM_EVENT_POOL_SIZE];

// Large event pool (events with larger data)
QF_MPOOL_EL(LargeEvt) l_lrgPoolSto[LARGE_EVENT_POOL_SIZE];

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
    
    // Enable local filters for Blinky AO
    QS_LOC_FILTER(QS_AO_OBJ, &AO_Blinky);
    
    // User-defined trace records
    QS_USR_DICTIONARY(QS_USER_00);   // LED state
    QS_USR_DICTIONARY(QS_USER_01);   // Button events
    QS_USR_DICTIONARY(QS_USER_02);   // Timing info
    QS_USR_DICTIONARY(QS_USER_03);   // Performance
    QS_USR_DICTIONARY(QS_USER_04);   // Reset events
    
    // Signal dictionary for readable trace output
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON_SIG, (void *)0);
    QS_SIG_DICTIONARY(TICK_SIG, (void *)0);
    
#endif // Q_SPY
    
    // Initialize and start Blinky Active Object
    Blinky_ctor();  // Constructor
    QACTIVE_START(&AO_Blinky,       // Active Object pointer
                  AO_BLINKY_PRIO,   // Priority
                  l_blinkyQueueSto, // Event queue storage
                  Q_DIM(l_blinkyQueueSto), // Queue length
                  (void *)0,        // Stack storage (not used in QK)
                  0U,               // Stack size (not used in QK)
                  (void *)0);       // Initialization parameter
    
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
    
    // Enable button interrupt
    NVIC_SetPriority(BUTTON_EXTI_IRQn, QF_AWARE_ISR_CMSIS_PRI + 2U);
    NVIC_EnableIRQ(BUTTON_EXTI_IRQn);
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
    if ((++idle_counter % 100000U) == 0U) {
        BSP_ledToggle(1);  // Heartbeat LED (LD3 - Orange)
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
    (void)prev;
    (void)next;
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
    BSP_ledOn(2);  // Error LED (LD5 - Red)
    
#ifdef Q_SPY
    // Log assertion failure
    QS_ASSERTION(module, loc, 10000U);
#endif
    
    // In debug mode, break into debugger
#ifdef DEBUG
    __BKPT(0);
#endif
    
    // In production, reset the system after a delay
    for (volatile uint32_t i = 0; i < 1000000U; ++i) {
        __NOP();
    }
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
    QACTIVE_PUBLISH(&tick_evt, &SysTick_Handler);
    
    // Process QF time events
    QTIMEEVT_TICK_X(0U, &SysTick_Handler);
    
    // Call BSP tick hook for application-specific processing
    BSP_tickHook();
    
    QK_ISR_EXIT();    // Inform QK kernel about ISR exit
}

//============================================================================
// BUTTON INTERRUPT HANDLER
//============================================================================

void EXTI0_IRQHandler(void) {
    // QK-aware interrupt entry
    QK_ISR_ENTRY();
    
    // Handle button interrupt
    if (__HAL_GPIO_EXTI_GET_IT(BUTTON_PIN) != 0x00U) {
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_PIN);
        
        // Post button event to system
        static QEvt const button_evt = QEVT_INITIALIZER(BUTTON_SIG);
        QACTIVE_PUBLISH(&button_evt, &EXTI0_IRQHandler);
    }
    
    // QK-aware interrupt exit
    QK_ISR_EXIT();
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
    // This is handled in BSP_init()
}

void QS_onCleanup(void) {
    // Cleanup QS resources
}

void QS_onFlush(void) {
    // Flush QS transmit buffer
    // Implementation is in BSP
}

QSTimeCtr QS_onGetTime(void) {
    // Return current timestamp for QS
    return (QSTimeCtr)BSP_getTimeUs();
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
            // Command 1: Reset blink counter
            Blinky_reset();
            break;
        }
        case 2U: {
            // Command 2: System reset
            NVIC_SystemReset();
            break;
        }
        default: {
            break;
        }
    }
}

#endif // Q_SPY