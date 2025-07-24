/**
 * @file {{AO_NAME_LOWER}}.h
 * @brief {{AO_DESCRIPTION}} Active Object Header
 * @version 1.0.0
 * @date {{GENERATION_DATE}}
 * 
 * Template for Active Object header files in QK-based projects
 * This template is used by AI agents to generate Active Objects
 * with proper QK kernel integration.
 */

#ifndef {{AO_NAME_UPPER}}_H
#define {{AO_NAME_UPPER}}_H

#include "qpc.h"
#include "project_template.h"

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
// ACTIVE OBJECT STRUCTURE
//============================================================================

/**
 * @brief {{AO_NAME}} Active Object structure
 * 
 * This structure contains all private data for the {{AO_NAME}} Active Object.
 * Following QK design principles:
 * - All data is private and encapsulated
 * - No shared state with other Active Objects
 * - Communication only through events
 */
typedef struct {
    QActive super;              /**< Inherit from QActive base class */
    
    // Time events for this Active Object
    QTimeEvt timeEvt;          /**< Periodic time event */
    QTimeEvt timeoutEvt;       /**< Timeout event */
    
    // Private state variables
    uint32_t counter;          /**< Internal counter */
    uint16_t state_data;       /**< State-specific data */
    uint8_t config_flags;      /**< Configuration flags */
    uint8_t error_count;       /**< Error counter */
    
    // QK-specific: No mutex/semaphore needed - event-driven design
    // All synchronization handled by framework
    
    // {{PRIVATE_DATA_MEMBERS}}
    
} {{AO_NAME}};

//============================================================================
// ACTIVE OBJECT INTERFACE
//============================================================================

/**
 * @brief Global Active Object instance
 * 
 * This is the single instance of the {{AO_NAME}} Active Object.
 * In QK, each Active Object is a singleton.
 */
extern {{AO_NAME}} AO_{{AO_NAME}};

/**
 * @brief Active Object Constructor
 * 
 * Initializes the {{AO_NAME}} Active Object.
 * Called once during system initialization.
 */
void {{AO_NAME}}_ctor(void);

/**
 * @brief Get Active Object Priority
 * 
 * @return Priority level for this Active Object
 */
uint_fast8_t {{AO_NAME}}_getPrio(void);

//============================================================================
// EVENT SIGNALS SPECIFIC TO THIS ACTIVE OBJECT
//============================================================================

/**
 * @brief {{AO_NAME}}-specific event signals
 * 
 * These signals are used for point-to-point communication
 * with this Active Object.
 */
enum {{AO_NAME}}Signals {
    {{AO_NAME_UPPER}}_START_SIG = {{SIGNAL_BASE}},  /**< Start operation */
    {{AO_NAME_UPPER}}_STOP_SIG,                     /**< Stop operation */
    {{AO_NAME_UPPER}}_CONFIG_SIG,                   /**< Configuration */
    {{AO_NAME_UPPER}}_TIMEOUT_SIG,                  /**< Timeout occurred */
    {{AO_NAME_UPPER}}_ERROR_SIG,                    /**< Error condition */
    
    // {{CUSTOM_SIGNALS}}
    
    {{AO_NAME_UPPER}}_MAX_SIG                       /**< Keep last */
};

//============================================================================
// EVENT STRUCTURES FOR THIS ACTIVE OBJECT
//============================================================================

/**
 * @brief Configuration event for {{AO_NAME}}
 */
typedef struct {
    QEvt super;                /**< Base event structure */
    uint16_t param_id;         /**< Parameter identifier */
    uint32_t value;            /**< Parameter value */
    uint8_t flags;             /**< Configuration flags */
} {{AO_NAME}}ConfigEvt;

/**
 * @brief Error event for {{AO_NAME}}
 */
typedef struct {
    QEvt super;                /**< Base event structure */
    uint16_t error_code;       /**< Error code */
    uint32_t error_data;       /**< Additional error information */
    const char *error_msg;     /**< Error message (for debugging) */
} {{AO_NAME}}ErrorEvt;

// {{CUSTOM_EVENT_STRUCTURES}}

//============================================================================
// STATE MACHINE DECLARATIONS
//============================================================================

/**
 * @brief State handler function declarations
 * 
 * These functions implement the hierarchical state machine
 * for the {{AO_NAME}} Active Object.
 */

// Top-level state
QState {{AO_NAME}}_initial({{AO_NAME}} * const me, QEvt const * const e);

// Main states
QState {{AO_NAME}}_inactive({{AO_NAME}} * const me, QEvt const * const e);
QState {{AO_NAME}}_active({{AO_NAME}} * const me, QEvt const * const e);
QState {{AO_NAME}}_error({{AO_NAME}} * const me, QEvt const * const e);

// Sub-states of active state
QState {{AO_NAME}}_idle({{AO_NAME}} * const me, QEvt const * const e);
QState {{AO_NAME}}_running({{AO_NAME}} * const me, QEvt const * const e);
QState {{AO_NAME}}_paused({{AO_NAME}} * const me, QEvt const * const e);

// {{CUSTOM_STATE_DECLARATIONS}}

//============================================================================
// QK-SPECIFIC PATTERNS AND CONSTRAINTS
//============================================================================

/**
 * @brief Maximum Run-to-Completion time for this AO
 * 
 * This defines the maximum time any event handler in this
 * Active Object should take to complete. Critical for QK
 * real-time performance.
 */
#define {{AO_NAME_UPPER}}_MAX_RTC_TIME_US    {{MAX_RTC_TIME}}U

/**
 * @brief Event queue depth
 * 
 * Size of the event queue for this Active Object.
 * Should be sized based on expected event arrival rate
 * and processing time.
 */
#define {{AO_NAME_UPPER}}_QUEUE_LEN         {{QUEUE_LENGTH}}U

/**
 * @brief Stack size (for QXK kernel only)
 * 
 * If using QXK kernel with extended tasks, this defines
 * the stack size. Not used with QV/QK kernels.
 */
#define {{AO_NAME_UPPER}}_STACK_SIZE        {{STACK_SIZE}}U

//============================================================================
// TIMING CONSTRAINTS (QK-SPECIFIC)
//============================================================================

/**
 * @brief Periodic timer intervals
 */
#define {{AO_NAME_UPPER}}_TICK_PERIOD_MS    {{TICK_PERIOD}}U
#define {{AO_NAME_UPPER}}_TIMEOUT_MS        {{TIMEOUT_VALUE}}U

/**
 * @brief Priority relationship constraints
 * 
 * Documents the priority relationships for Rate Monotonic Analysis.
 * Higher frequency tasks should have higher priority in QK.
 */
#define {{AO_NAME_UPPER}}_FREQUENCY_HZ      {{FREQUENCY}}U
#define {{AO_NAME_UPPER}}_DEADLINE_MS       {{DEADLINE}}U

//============================================================================
// DEBUGGING AND TESTING SUPPORT
//============================================================================

#ifdef Q_SPY
/**
 * @brief QS trace records for this Active Object
 */
enum {{AO_NAME}}TraceRecords {
    {{AO_NAME_UPPER}}_STATE_ENTRY = QS_USER_00 + {{TRACE_OFFSET}},
    {{AO_NAME_UPPER}}_STATE_EXIT,
    {{AO_NAME_UPPER}}_EVENT_RECEIVED,
    {{AO_NAME_UPPER}}_ERROR_DETECTED,
    {{AO_NAME_UPPER}}_PERFORMANCE_INFO,
    
    // {{CUSTOM_TRACE_RECORDS}}
};

/**
 * @brief Performance monitoring macros
 */
#define {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(state) \
    QS_BEGIN_ID({{AO_NAME_UPPER}}_STATE_ENTRY, AO_{{AO_NAME}}->prio) \
        QS_STR_(#state); \
        QS_TIME_(); \
    QS_END_()

#define {{AO_NAME_UPPER}}_TRACE_EVENT(sig) \
    QS_BEGIN_ID({{AO_NAME_UPPER}}_EVENT_RECEIVED, AO_{{AO_NAME}}->prio) \
        QS_SIG_(sig); \
        QS_TIME_(); \
    QS_END_()

#else
#define {{AO_NAME_UPPER}}_TRACE_STATE_ENTRY(state) ((void)0)
#define {{AO_NAME_UPPER}}_TRACE_EVENT(sig) ((void)0)
#endif // Q_SPY

//============================================================================
// UNIT TESTING SUPPORT
//============================================================================

#ifdef UNIT_TEST
/**
 * @brief Test interface for accessing private data
 * 
 * These functions are only available during unit testing
 * to allow verification of internal state.
 */
uint32_t {{AO_NAME}}_test_getCounter(void);
uint16_t {{AO_NAME}}_test_getStateData(void);
void {{AO_NAME}}_test_setState(QStateHandler state);
void {{AO_NAME}}_test_injectEvent(QEvt const * const e);

// {{TEST_INTERFACE_FUNCTIONS}}

#endif // UNIT_TEST

//============================================================================
// TEMPLATE EXPANSION MARKERS
//============================================================================

// These markers are used by code generation tools to insert
// project-specific code:

// {{ADDITIONAL_INCLUDES}}
// {{CUSTOM_MACROS}}
// {{PLATFORM_SPECIFIC_DECLARATIONS}}
// {{PROJECT_SPECIFIC_INTERFACE}}

#ifdef __cplusplus
}
#endif

#endif // {{AO_NAME_UPPER}}_H

/**
 * @brief Template Usage Instructions for AI Agents
 * 
 * This template should be used as follows:
 * 
 * 1. Replace all {{TEMPLATE_VARIABLES}} with actual values
 * 2. Customize the private data members based on requirements
 * 3. Define appropriate event signals and structures
 * 4. Implement the state machine in the corresponding .c file
 * 5. Set appropriate timing constraints based on requirements
 * 6. Configure QS tracing for debugging
 * 
 * QK-Specific Considerations:
 * - All event handlers must complete in bounded time
 * - No blocking operations allowed
 * - Priority assignment should follow RMA principles
 * - Event queue sizing should consider worst-case scenarios
 * - Memory allocation should be static/pool-based
 * 
 * Template Variables to Replace:
 * - {{AO_NAME_UPPER}}: Active Object name in uppercase
 * - {{AO_NAME_LOWER}}: Active Object name in lowercase  
 * - {{AO_NAME}}: Active Object name in proper case
 * - {{AO_DESCRIPTION}}: Brief description of the AO's purpose
 * - {{GENERATION_DATE}}: Date of code generation
 * - {{SIGNAL_BASE}}: Base signal number for this AO
 * - {{MAX_RTC_TIME}}: Maximum run-to-completion time in microseconds
 * - {{QUEUE_LENGTH}}: Event queue depth
 * - {{STACK_SIZE}}: Stack size (QXK only)
 * - {{TICK_PERIOD}}: Periodic timer period in milliseconds
 * - {{TIMEOUT_VALUE}}: Timeout value in milliseconds
 * - {{FREQUENCY}}: Operating frequency in Hz
 * - {{DEADLINE}}: Deadline in milliseconds
 * - {{TRACE_OFFSET}}: Offset for QS trace records
 */