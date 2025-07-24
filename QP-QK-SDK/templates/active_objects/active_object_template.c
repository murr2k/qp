/**
 * @file {{AO_NAME_LOWER}}.c
 * @brief {{AO_DESCRIPTION}} Active Object Implementation
 * @version 1.0.0
 * @date {{GENERATION_DATE}}
 * 
 * Template for Active Object implementation files in QK-based projects
 * This template is used by AI agents to generate Active Objects
 * with proper QK kernel integration and hierarchical state machines.
 */

#include "{{AO_NAME_LOWER}}.h"

Q_DEFINE_THIS_FILE

//============================================================================
// LOCAL CONSTANTS AND MACROS
//============================================================================

// Internal timing constants
#define PERIODIC_TIMEOUT_TICKS  (BSP_TICKS_PER_SEC / {{FREQUENCY}})
#define WATCHDOG_TIMEOUT_TICKS  (BSP_TICKS_PER_SEC * {{TIMEOUT_VALUE}} / 1000U)

// Internal state flags
#define FLAG_INITIALIZED        (1U << 0)
#define FLAG_CONFIGURED         (1U << 1)
#define FLAG_ERROR_STATE        (1U << 2)
#define FLAG_TIMEOUT_ACTIVE     (1U << 3)

// {{LOCAL_CONSTANTS}}

//============================================================================
// ACTIVE OBJECT INSTANCE
//============================================================================

/**
 * @brief Global instance of the {{AO_NAME}} Active Object
 * 
 * This is the single instance of the Active Object, following
 * the singleton pattern required by the QP framework.
 */
{{AO_NAME}} AO_{{AO_NAME}};

//============================================================================
// LOCAL FUNCTION PROTOTYPES
//============================================================================

// Helper functions for internal operations
static void {{AO_NAME}}_initializeHardware({{AO_NAME}} * const me);
static void {{AO_NAME}}_startPeriodicTimer({{AO_NAME}} * const me);
static void {{AO_NAME}}_stopPeriodicTimer({{AO_NAME}} * const me);
static void {{AO_NAME}}_handleError({{AO_NAME}} * const me, uint16_t error_code);
static bool {{AO_NAME}}_validateConfig({{AO_NAME}}ConfigEvt const * const e);

// {{LOCAL_FUNCTION_PROTOTYPES}}

//============================================================================
// ACTIVE OBJECT CONSTRUCTOR
//============================================================================

void {{AO_NAME}}_ctor(void) {
    {{AO_NAME}} * const me = &AO_{{AO_NAME}};
    
    // Initialize the Active Object base class
    QActive_ctor(&me->super, Q_STATE_CAST(&{{AO_NAME}}_initial));
    
    // Initialize time events
    QTimeEvt_ctorX(&me->timeEvt, &me->super, {{AO_NAME_UPPER}}_TIMEOUT_SIG, 0U);
    QTimeEvt_ctorX(&me->timeoutEvt, &me->super, {{AO_NAME_UPPER}}_TIMEOUT_SIG, 0U);
    
    // Initialize private data members
    me->counter = 0U;
    me->state_data = 0U;
    me->config_flags = 0U;
    me->error_count = 0U;
    
    // {{CONSTRUCTOR_INITIALIZATION}}
}

//============================================================================
// ACTIVE OBJECT INTERFACE FUNCTIONS
//============================================================================

uint_fast8_t {{AO_NAME}}_getPrio(void) {
    return AO_{{AO_NAME_UPPER}}_PRIO;
}

//============================================================================
// HIERARCHICAL STATE MACHINE IMPLEMENTATION
//============================================================================

/**
 * @brief Initial transition for {{AO_NAME}} Active Object
 * 
 * This is called once when the Active Object starts.
 * It performs initialization and transitions to the operational state.
 */
QState {{AO_NAME}}_initial({{AO_NAME}} * const me, QEvt const * const e) {
    (void)e; // Unused parameter
    
    {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(initial);
    
    // Subscribe to published events that this AO needs
    QActive_subscribe(&me->super, TICK_SIG);
    QActive_subscribe(&me->super, FAULT_SIG);
    QActive_subscribe(&me->super, MODE_CHANGE_SIG);
    
    // {{SUBSCRIPTION_LIST}}
    
    // Initialize hardware interfaces
    {{AO_NAME}}_initializeHardware(me);
    
    // Set initialization flag
    me->config_flags |= FLAG_INITIALIZED;
    
    // Perform initial transition to inactive state
    return Q_TRAN(&{{AO_NAME}}_inactive);
}

/**
 * @brief Inactive state - waiting for start command
 * 
 * In this state, the Active Object is initialized but not
 * performing its main function. It waits for a start command.
 */
QState {{AO_NAME}}_inactive({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(inactive);
            
            // Entry actions for inactive state
            me->counter = 0U;
            me->state_data = 0U;
            
            // {{INACTIVE_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for inactive state
            // {{INACTIVE_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_START_SIG: {
            {{AO_NAME_UPPER}}_TRACE_EVENT({{AO_NAME_UPPER}}_START_SIG);
            
            // Transition to active state
            status_ = Q_TRAN(&{{AO_NAME}}_active);
            break;
        }
        
        case {{AO_NAME_UPPER}}_CONFIG_SIG: {
            {{AO_NAME_UPPER}}_TRACE_EVENT({{AO_NAME_UPPER}}_CONFIG_SIG);
            
            // Handle configuration in inactive state
            {{AO_NAME}}ConfigEvt const *cfg = Q_EVT_CAST({{AO_NAME}}ConfigEvt);
            
            if ({{AO_NAME}}_validateConfig(cfg)) {
                // Apply configuration
                me->state_data = cfg->value;
                me->config_flags |= FLAG_CONFIGURED;
            } else {
                // Invalid configuration
                {{AO_NAME}}_handleError(me, ERROR_INVALID_PARAM);
            }
            
            status_ = Q_HANDLED();
            break;
        }
        
        case FAULT_SIG: {
            // Global fault signal - transition to error state
            status_ = Q_TRAN(&{{AO_NAME}}_error);
            break;
        }
        
        default: {
            // Delegate unhandled events to superstate
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    
    return status_;
}

/**
 * @brief Active state - main operational state
 * 
 * This is the main operational state with sub-states for
 * different operating modes (idle, running, paused).
 */
QState {{AO_NAME}}_active({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(active);
            
            // Entry actions for active state
            {{AO_NAME}}_startPeriodicTimer(me);
            
            // {{ACTIVE_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for active state
            {{AO_NAME}}_stopPeriodicTimer(me);
            
            // {{ACTIVE_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_INIT_SIG: {
            // Initial transition within active state
            status_ = Q_TRAN(&{{AO_NAME}}_idle);
            break;
        }
        
        case {{AO_NAME_UPPER}}_STOP_SIG: {
            {{AO_NAME_UPPER}}_TRACE_EVENT({{AO_NAME_UPPER}}_STOP_SIG);
            
            // Return to inactive state
            status_ = Q_TRAN(&{{AO_NAME}}_inactive);
            break;
        }
        
        case {{AO_NAME_UPPER}}_CONFIG_SIG: {
            {{AO_NAME_UPPER}}_TRACE_EVENT({{AO_NAME_UPPER}}_CONFIG_SIG);
            
            // Handle configuration in active state
            {{AO_NAME}}ConfigEvt const *cfg = Q_EVT_CAST({{AO_NAME}}ConfigEvt);
            
            if ({{AO_NAME}}_validateConfig(cfg)) {
                // Apply configuration without state change
                me->state_data = cfg->value;
                me->config_flags |= FLAG_CONFIGURED;
            } else {
                {{AO_NAME}}_handleError(me, ERROR_INVALID_PARAM);
            }
            
            status_ = Q_HANDLED();
            break;
        }
        
        case TICK_SIG: {
            // System tick - increment counter
            me->counter++;
            
            // Check for periodic processing
            if ((me->counter % 100U) == 0U) {
                // Periodic processing every 100 ticks
                // {{PERIODIC_PROCESSING}}
            }
            
            status_ = Q_HANDLED();
            break;
        }
        
        case FAULT_SIG: {
            // Global fault signal
            status_ = Q_TRAN(&{{AO_NAME}}_error);
            break;
        }
        
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    
    return status_;
}

/**
 * @brief Idle sub-state of active state
 * 
 * The Active Object is active but not performing specific tasks.
 * It's ready to start operations when commanded.
 */
QState {{AO_NAME}}_idle({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(idle);
            
            // Entry actions for idle sub-state
            // {{IDLE_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for idle sub-state
            // {{IDLE_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_START_SIG: {
            // Start main operation
            status_ = Q_TRAN(&{{AO_NAME}}_running);
            break;
        }
        
        default: {
            // Delegate to parent state
            status_ = Q_SUPER(&{{AO_NAME}}_active);
            break;
        }
    }
    
    return status_;
}

/**
 * @brief Running sub-state of active state
 * 
 * The Active Object is performing its main function.
 * This is where the primary work gets done.
 */
QState {{AO_NAME}}_running({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(running);
            
            // Entry actions for running sub-state
            // {{RUNNING_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for running sub-state
            // {{RUNNING_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_TIMEOUT_SIG: {
            // Timeout event from periodic timer
            {{AO_NAME_UPPER}}_TRACE_EVENT({{AO_NAME_UPPER}}_TIMEOUT_SIG);
            
            // Perform main operation
            // {{MAIN_OPERATION_CODE}}
            
            // Restart the timer for next cycle
            QTimeEvt_armX(&me->timeEvt, PERIODIC_TIMEOUT_TICKS, 0U);
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_START_SIG: {
            // Already running - ignore duplicate start
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_STOP_SIG: {
            // Pause operation
            status_ = Q_TRAN(&{{AO_NAME}}_paused);
            break;
        }
        
        default: {
            status_ = Q_SUPER(&{{AO_NAME}}_active);
            break;
        }
    }
    
    return status_;
}

/**
 * @brief Paused sub-state of active state
 * 
 * The Active Object is temporarily suspended but ready
 * to resume operation when commanded.
 */
QState {{AO_NAME}}_paused({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(paused);
            
            // Entry actions for paused sub-state
            // Stop periodic processing
            QTimeEvt_disarm(&me->timeEvt);
            
            // {{PAUSED_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for paused sub-state
            // {{PAUSED_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_START_SIG: {
            // Resume operation
            status_ = Q_TRAN(&{{AO_NAME}}_running);
            break;
        }
        
        case {{AO_NAME_UPPER}}_STOP_SIG: {
            // Go to idle state
            status_ = Q_TRAN(&{{AO_NAME}}_idle);
            break;
        }
        
        default: {
            status_ = Q_SUPER(&{{AO_NAME}}_active);
            break;
        }
    }
    
    return status_;
}

/**
 * @brief Error state - handles fault conditions
 * 
 * The Active Object enters this state when an error occurs.
 * It attempts recovery or waits for external intervention.
 */
QState {{AO_NAME}}_error({{AO_NAME}} * const me, QEvt const * const e) {
    QState status_;
    
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(error);
            
            // Entry actions for error state
            me->config_flags |= FLAG_ERROR_STATE;
            me->error_count++;
            
            // Start watchdog timer for recovery attempt
            QTimeEvt_armX(&me->timeoutEvt, WATCHDOG_TIMEOUT_TICKS, 0U);
            
            // {{ERROR_ENTRY_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case Q_EXIT_SIG: {
            // Exit actions for error state
            me->config_flags &= ~FLAG_ERROR_STATE;
            QTimeEvt_disarm(&me->timeoutEvt);
            
            // {{ERROR_EXIT_ACTIONS}}
            
            status_ = Q_HANDLED();
            break;
        }
        
        case {{AO_NAME_UPPER}}_TIMEOUT_SIG: {
            // Watchdog timeout - attempt recovery
            if (me->error_count < 3U) {
                // Try to recover
                status_ = Q_TRAN(&{{AO_NAME}}_inactive);
            } else {
                // Too many errors - stay in error state
                // Reset error count for next attempt
                me->error_count = 0U;
                QTimeEvt_armX(&me->timeoutEvt, WATCHDOG_TIMEOUT_TICKS, 0U);
                status_ = Q_HANDLED();
            }
            break;
        }
        
        case {{AO_NAME_UPPER}}_START_SIG: {
            // Manual recovery attempt
            me->error_count = 0U;
            status_ = Q_TRAN(&{{AO_NAME}}_inactive);
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

/**
 * @brief Initialize hardware interfaces for this Active Object
 */
static void {{AO_NAME}}_initializeHardware({{AO_NAME}} * const me) {
    (void)me; // May be unused
    
    // Initialize hardware-specific interfaces
    // {{HARDWARE_INITIALIZATION}}
}

/**
 * @brief Start the periodic timer for this Active Object
 */
static void {{AO_NAME}}_startPeriodicTimer({{AO_NAME}} * const me) {
    // Arm the periodic time event
    QTimeEvt_armX(&me->timeEvt, PERIODIC_TIMEOUT_TICKS, PERIODIC_TIMEOUT_TICKS);
}

/**
 * @brief Stop the periodic timer for this Active Object
 */
static void {{AO_NAME}}_stopPeriodicTimer({{AO_NAME}} * const me) {
    // Disarm the periodic time event
    QTimeEvt_disarm(&me->timeEvt);
}

/**
 * @brief Handle error conditions
 */
static void {{AO_NAME}}_handleError({{AO_NAME}} * const me, uint16_t error_code) {
    // Log the error
#ifdef Q_SPY
    QS_BEGIN_ID({{AO_NAME_UPPER}}_ERROR_DETECTED, AO_{{AO_NAME}}->prio)
        QS_U16_(error_code);
        QS_TIME_();
    QS_END_()
#endif
    
    // Create and post error event to self
    {{AO_NAME}}ErrorEvt *err_evt = Q_NEW({{AO_NAME}}ErrorEvt, {{AO_NAME_UPPER}}_ERROR_SIG);
    err_evt->error_code = error_code;
    err_evt->error_data = me->counter;
    err_evt->error_msg = "{{AO_NAME}} Error";
    
    QACTIVE_POST(&me->super, &err_evt->super, &me->super);
}

/**
 * @brief Validate configuration parameters
 */
static bool {{AO_NAME}}_validateConfig({{AO_NAME}}ConfigEvt const * const e) {
    bool valid = true;
    
    // Validate configuration parameters
    if (e->param_id > 100U) {
        valid = false;
    }
    
    if (e->value == 0U) {
        valid = false;
    }
    
    // {{CONFIGURATION_VALIDATION}}
    
    return valid;
}

//============================================================================
// UNIT TESTING INTERFACE
//============================================================================

#ifdef UNIT_TEST

uint32_t {{AO_NAME}}_test_getCounter(void) {
    return AO_{{AO_NAME}}.counter;
}

uint16_t {{AO_NAME}}_test_getStateData(void) {
    return AO_{{AO_NAME}}.state_data;
}

void {{AO_NAME}}_test_setState(QStateHandler state) {
    QHsm_tran(&AO_{{AO_NAME}}.super.super, state);
}

void {{AO_NAME}}_test_injectEvent(QEvt const * const e) {
    QACTIVE_POST(&AO_{{AO_NAME}}.super, e, &AO_{{AO_NAME}}.super);
}

// {{TEST_INTERFACE_IMPLEMENTATION}}

#endif // UNIT_TEST

//============================================================================
// TEMPLATE EXPANSION MARKERS
//============================================================================

// These markers are used by code generation tools:
// {{ADDITIONAL_HELPER_FUNCTIONS}}
// {{PLATFORM_SPECIFIC_IMPLEMENTATION}}
// {{PROJECT_SPECIFIC_CODE}}