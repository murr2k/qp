/**
 * @file blinky.c
 * @brief Blinky Active Object Implementation
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Simple LED blinking Active Object demonstrating basic QP/QK concepts.
 * This example shows:
 * - Single Active Object with hierarchical state machine
 * - Periodic timer events
 * - QK preemptive scheduling
 * - QS software tracing integration
 */

#include "blinky.h"

Q_DEFINE_THIS_FILE

//============================================================================
// LOCAL CONSTANTS
//============================================================================

// Blink timing in system ticks (500ms for 1Hz blink)
#define BLINK_PERIOD_TICKS      (BSP_TICKS_PER_SEC / 2U)

//============================================================================
// GLOBAL OBJECTS
//============================================================================

// The single instance of the Blinky Active Object
Blinky AO_Blinky;

//============================================================================
// LOCAL FUNCTION PROTOTYPES
//============================================================================

static void Blinky_performBlink(Blinky * const me, uint8_t led_state);

//============================================================================
// ACTIVE OBJECT CONSTRUCTOR
//============================================================================

void Blinky_ctor(void) {
    Blinky * const me = &AO_Blinky;
    
    // Initialize the Active Object base class
    QActive_ctor(&me->super, Q_STATE_CAST(&Blinky_initial));
    
    // Initialize time event for periodic blinking
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
    
    // Initialize private data
    me->blink_count = 0U;
    me->is_running = false;
}

//============================================================================
// HIERARCHICAL STATE MACHINE IMPLEMENTATION
//============================================================================

QState Blinky_initial(Blinky * const me, QEvt const * const e) {
    (void)e; // Unused parameter
    
    // Subscribe to published events
    QActive_subscribe(&me->super, BUTTON_SIG);
    QActive_subscribe(&me->super, TICK_SIG);
    
    // QS trace
    QS_OBJ_DICTIONARY(&AO_Blinky);
    QS_OBJ_DICTIONARY(&AO_Blinky.timeEvt);
    
    QS_SIG_DICTIONARY(TIMEOUT_SIG, &AO_Blinky);
    QS_SIG_DICTIONARY(BUTTON_SIG, &AO_Blinky);
    
    QS_FUN_DICTIONARY(&Blinky_initial);
    QS_FUN_DICTIONARY(&Blinky_off);
    QS_FUN_DICTIONARY(&Blinky_on);
    
    // Start in the "off" state
    return Q_TRAN(&Blinky_off);
}

QState Blinky_off(Blinky * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Entry action: turn off LED and start timer
            BSP_ledOff(0);  // Main blink LED (LD2)
            me->is_running = true;
            
            // Arm the timer for next blink
            QTimeEvt_armX(&me->timeEvt, BLINK_PERIOD_TICKS, 0U);
            
            // QS trace
            QS_BEGIN_ID(QS_USER_00, AO_Blinky.super.prio)
                QS_STR_("LED OFF");
                QS_U32_(me->blink_count);
            QS_END_()
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit action: disarm the timer
            QTimeEvt_disarm(&me->timeEvt);
            status_ = Q_HANDLED();
            break;
        }
        
        case TIMEOUT_SIG: {
            // Timeout expired, transition to "on" state
            status_ = Q_TRAN(&Blinky_on);
            break;
        }
        
        case BUTTON_SIG: {
            // Button pressed - toggle error LED
            BSP_ledToggle(3);  // Error LED (LD5)
            
            // QS trace
            QS_BEGIN_ID(QS_USER_01, AO_Blinky.super.prio)
                QS_STR_("BUTTON");
                QS_U8_(0);  // Pressed in OFF state
            QS_END_()
            
            status_ = Q_HANDLED();
            break;
        }
        
        case TICK_SIG: {
            // System tick - count ticks (every 10th tick)
            if ((me->blink_count % 10U) == 0U) {
                // Periodic processing in OFF state
                // Could be used for power management, etc.
            }
            status_ = Q_HANDLED();
            break;
        }
        
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    
    return status_;
}

QState Blinky_on(Blinky * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Entry action: turn on LED and start timer
            BSP_ledOn(0);   // Main blink LED (LD2)
            me->blink_count++;
            
            // Arm the timer for next blink
            QTimeEvt_armX(&me->timeEvt, BLINK_PERIOD_TICKS, 0U);
            
            // QS trace
            QS_BEGIN_ID(QS_USER_00, AO_Blinky.super.prio)
                QS_STR_("LED ON");
                QS_U32_(me->blink_count);
            QS_END_()
            
            // Performance demonstration: measure entry time
            uint32_t entry_time = BSP_getTimeUs();
            QS_BEGIN_ID(QS_USER_02, AO_Blinky.super.prio)
                QS_STR_("ENTRY_TIME");
                QS_U32_(entry_time);
            QS_END_()
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit action: disarm the timer
            QTimeEvt_disarm(&me->timeEvt);
            status_ = Q_HANDLED();
            break;
        }
        
        case TIMEOUT_SIG: {
            // Timeout expired, transition to "off" state
            status_ = Q_TRAN(&Blinky_off);
            break;
        }
        
        case BUTTON_SIG: {
            // Button pressed - toggle error LED
            BSP_ledToggle(3);  // Error LED (LD5)
            
            // QS trace
            QS_BEGIN_ID(QS_USER_01, AO_Blinky.super.prio)
                QS_STR_("BUTTON");
                QS_U8_(1);  // Pressed in ON state
            QS_END_()
            
            status_ = Q_HANDLED();
            break;
        }
        
        case TICK_SIG: {
            // System tick - performance monitoring
            // Every 1000 ticks (1 second), report statistics
            static uint32_t tick_counter = 0U;
            
            if ((++tick_counter % 1000U) == 0U) {
                // Report performance statistics
                QS_BEGIN_ID(QS_USER_03, AO_Blinky.super.prio)
                    QS_STR_("PERFORMANCE");
                    QS_U32_(me->blink_count);
                    QS_U32_(tick_counter);
                QS_END_()
            }
            
            status_ = Q_HANDLED();
            break;
        }
        
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    
    return status_;
}

//============================================================================
// LOCAL HELPER FUNCTIONS
//============================================================================

static void Blinky_performBlink(Blinky * const me, uint8_t led_state) {
    // Helper function for LED control with tracing
    if (led_state != 0U) {
        BSP_ledOn(0);
    } else {
        BSP_ledOff(0);
    }
    
    // Update blink count
    if (led_state != 0U) {
        me->blink_count++;
    }
    
    // QS trace
    QS_BEGIN_ID(QS_USER_00, me->super.prio)
        QS_STR_(led_state ? "LED ON" : "LED OFF");
        QS_U32_(me->blink_count);
    QS_END_()
}

//============================================================================
// INTERFACE FUNCTIONS
//============================================================================

uint32_t Blinky_getBlinkCount(void) {
    return AO_Blinky.blink_count;
}

bool Blinky_isRunning(void) {
    return AO_Blinky.is_running;
}

void Blinky_reset(void) {
    Blinky * const me = &AO_Blinky;
    
    // Reset blink counter
    me->blink_count = 0U;
    
    // QS trace
    QS_BEGIN_ID(QS_USER_04, me->super.prio)
        QS_STR_("RESET");
    QS_END_()
}