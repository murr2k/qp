/**
 * @file blinky.h
 * @brief Blinky Active Object Header
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Header file for the Blinky Active Object example.
 * Demonstrates basic QP/QK Active Object structure and interface.
 */

#ifndef BLINKY_H
#define BLINKY_H

#include "qpc.h"
#include "project_config.h"

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
// EVENT SIGNALS
//============================================================================

enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG,   // Timer timeout signal
    BUTTON_SIG,                 // Button press signal
    TICK_SIG,                   // System tick signal
    MAX_SIG                     // Keep last
};

//============================================================================
// ACTIVE OBJECT STRUCTURE
//============================================================================

/**
 * @brief Blinky Active Object structure
 * 
 * The Blinky Active Object encapsulates all data and behavior
 * for the LED blinking functionality.
 */
typedef struct {
    QActive super;              /**< Inherit from QActive base class */
    
    QTimeEvt timeEvt;          /**< Timer event for periodic blinking */
    
    uint32_t blink_count;      /**< Number of blinks performed */
    bool is_running;           /**< Running state flag */
    
} Blinky;

//============================================================================
// GLOBAL OBJECT DECLARATION
//============================================================================

/**
 * @brief Global Blinky Active Object instance
 */
extern Blinky AO_Blinky;

//============================================================================
// PUBLIC INTERFACE
//============================================================================

/**
 * @brief Blinky Active Object constructor
 * 
 * Initializes the Blinky Active Object. Must be called once
 * during system initialization before starting the AO.
 */
void Blinky_ctor(void);

/**
 * @brief Get current blink count
 * 
 * @return Number of blinks performed since startup or last reset
 */
uint32_t Blinky_getBlinkCount(void);

/**
 * @brief Check if Blinky is currently running
 * 
 * @return true if actively blinking, false otherwise
 */
bool Blinky_isRunning(void);

/**
 * @brief Reset blink counter
 * 
 * Resets the internal blink counter to zero.
 */
void Blinky_reset(void);

//============================================================================
// STATE MACHINE DECLARATIONS
//============================================================================

/**
 * @brief State handler function declarations
 */
QState Blinky_initial(Blinky * const me, QEvt const * const e);
QState Blinky_off(Blinky * const me, QEvt const * const e);
QState Blinky_on(Blinky * const me, QEvt const * const e);

//============================================================================
// CONFIGURATION
//============================================================================

/**
 * @brief Blinky Active Object priority
 * 
 * Priority level for the Blinky AO in the QK scheduler.
 * Higher number = higher priority.
 */
#define AO_BLINKY_PRIO          3U

/**
 * @brief Event queue depth
 * 
 * Maximum number of events that can be queued for the Blinky AO.
 */
#define BLINKY_QUEUE_LEN        10U

/**
 * @brief QS trace records for Blinky
 * 
 * User-defined trace record IDs for debugging and analysis.
 */
enum BlinkyTraceRecords {
    BLINKY_LED_STATE = QS_USER,    /**< LED state change */
    BLINKY_BUTTON_PRESS,            /**< Button press event */
    BLINKY_ENTRY_TIME,              /**< State entry timing */
    BLINKY_PERFORMANCE,             /**< Performance statistics */
    BLINKY_RESET                    /**< Reset operation */
};

#ifdef __cplusplus
}
#endif

#endif // BLINKY_H