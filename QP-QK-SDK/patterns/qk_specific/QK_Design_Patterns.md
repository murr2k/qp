# QK Kernel Design Patterns and Best Practices

## Overview

This document outlines design patterns and best practices specifically for the QK (preemptive non-blocking) kernel in the Quantum Platform framework. These patterns are optimized for AI agents developing embedded firmware with real-time constraints.

## QK Kernel Characteristics

### Key Features
- **Preemptive scheduling**: Higher priority Active Objects can interrupt lower priority ones
- **Single stack architecture**: All Active Objects share one stack, saving significant RAM
- **Non-blocking execution**: All Active Objects must use Run-to-Completion (RTC) semantics
- **Priority-based**: Up to 64 priority levels supported
- **Efficient context switching**: Implemented as function calls, not full thread switches

### Performance Benefits
- **Memory efficient**: ~4-6KB ROM overhead, minimal RAM per Active Object
- **Fast preemption**: Context switch in <5μs on ARM Cortex-M3 @ 72MHz
- **Deterministic**: Predictable response times for real-time systems
- **Low overhead**: <3% CPU overhead typical

## Priority Assignment Patterns

### Rate Monotonic Analysis (RMA) Pattern
```c
// Priority assignment based on execution frequency
// Higher frequency = Higher priority in QK

enum ActiveObjectPriorities {
    // Lowest priority - background tasks
    AO_LOGGER_PRIO = 1U,        // 0.1 Hz - data logging
    AO_COMM_PRIO = 2U,          // 1 Hz - communication
    
    // Medium priority - application tasks  
    AO_SENSOR_PRIO = 3U,        // 10 Hz - sensor reading
    AO_CONTROLLER_PRIO = 4U,    // 50 Hz - control loop
    
    // High priority - time-critical tasks
    AO_MOTOR_PRIO = 5U,         // 100 Hz - motor control
    AO_ADC_PRIO = 6U,           // 1000 Hz - ADC sampling
    
    // Highest priority - interrupt-like
    AO_SAFETY_PRIO = 7U,        // Emergency response
};

// Priority assignment validation
#define VALIDATE_PRIORITY_ORDER() do { \
    Q_ASSERT(AO_LOGGER_PRIO < AO_COMM_PRIO); \
    Q_ASSERT(AO_COMM_PRIO < AO_SENSOR_PRIO); \
    /* ... continue for all priorities */ \
} while(0)
```

### Deadline Monotonic Pattern
```c
// Alternative: assign priority based on deadlines
// Shorter deadline = Higher priority

typedef struct {
    uint8_t priority;
    uint32_t period_ms;
    uint32_t deadline_ms;
    uint32_t wcet_us;    // Worst Case Execution Time
} TaskCharacteristics;

static const TaskCharacteristics task_table[] = {
    {AO_SAFETY_PRIO,     10,   5, 100},   // 5ms deadline
    {AO_MOTOR_PRIO,      20,  15, 500},   // 15ms deadline  
    {AO_CONTROLLER_PRIO, 100, 80, 2000},  // 80ms deadline
    {AO_SENSOR_PRIO,     100, 90, 1000},  // 90ms deadline
};

// Schedulability test
bool validate_schedulability(void) {
    float utilization = 0.0f;
    for (int i = 0; i < Q_DIM(task_table); i++) {
        utilization += (float)task_table[i].wcet_us / 
                      (task_table[i].period_ms * 1000.0f);
    }
    return utilization <= 0.69f; // RMA bound for 4 tasks
}
```

## Event Handling Patterns

### Run-to-Completion Pattern
```c
// CORRECT: All event handlers must complete quickly
QState MyAO_running(MyAO * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case SENSOR_DATA_SIG: {
            SensorEvt const *se = Q_EVT_CAST(SensorEvt);
            
            // Quick processing only
            me->sensor_value = se->value;
            me->processed_count++;
            
            // Post result to another AO if needed
            ProcessedEvt *pe = Q_NEW(ProcessedEvt, PROCESSED_SIG);
            pe->result = me->sensor_value * 2;
            QACTIVE_POST(AO_Display, &pe->super, me);
            
            status_ = Q_HANDLED();
            break;
        }
    }
    return status_;
}

// INCORRECT: Never block in QK
QState MyAO_wrong(MyAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case PROCESS_SIG: {
            // WRONG: Blocking delay
            HAL_Delay(100);  // Blocks entire system!
            
            // WRONG: Polling loop
            while (!(UART->SR & UART_SR_TXE)) {
                // Busy waiting blocks preemption
            }
            
            // WRONG: Synchronous I/O
            result = blocking_i2c_read();  // Blocks other AOs
            
            break;
        }
    }
    return Q_HANDLED();
}
```

### Asynchronous Processing Pattern
```c
// Convert blocking operations to asynchronous events
QState MyAO_start_operation(MyAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case START_I2C_READ_SIG: {
            // Start asynchronous I2C read
            I2C_start_read_async(DEVICE_ADDR, buffer, sizeof(buffer));
            
            // Transition to waiting state
            return Q_TRAN(&MyAO_waiting_i2c);
        }
    }
    return Q_SUPER(&MyAO_active);
}

QState MyAO_waiting_i2c(MyAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case I2C_COMPLETE_SIG: {
            // I2C operation completed (from ISR)
            I2cCompleteEvt const *ice = Q_EVT_CAST(I2cCompleteEvt);
            
            if (ice->status == I2C_SUCCESS) {
                // Process the received data
                process_i2c_data(ice->data, ice->length);
                return Q_TRAN(&MyAO_processing);
            } else {
                // Handle error
                return Q_TRAN(&MyAO_error);
            }
        }
        
        case TIMEOUT_SIG: {
            // I2C operation timed out
            I2C_abort();
            return Q_TRAN(&MyAO_error);
        }
    }
    return Q_SUPER(&MyAO_active);
}
```

## Timing and Synchronization Patterns

### Periodic Task Pattern
```c
// Use QTimeEvt for periodic operations
typedef struct {
    QActive super;
    QTimeEvt periodicEvt;
    uint32_t cycle_count;
} PeriodicAO;

QState PeriodicAO_active(PeriodicAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Start periodic timer on entry
            QTimeEvt_armX(&me->periodicEvt, 
                         BSP_TICKS_PER_SEC / FREQ_HZ,     // Initial delay
                         BSP_TICKS_PER_SEC / FREQ_HZ);    // Period
            return Q_HANDLED();
        }
        
        case Q_EXIT_SIG: {
            // Stop timer on exit
            QTimeEvt_disarm(&me->periodicEvt);
            return Q_HANDLED();
        }
        
        case PERIODIC_SIG: {
            // Periodic processing
            me->cycle_count++;
            
            // Perform time-critical work here
            perform_control_algorithm();
            
            // Monitor timing
            if (me->cycle_count % 1000U == 0U) {
                // Every 1000 cycles, check timing
                check_timing_constraints();
            }
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

### Timeout Pattern
```c
// Implement timeouts for fault tolerance
QState MyAO_waiting(MyAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Start timeout timer
            QTimeEvt_armX(&me->timeoutEvt, TIMEOUT_TICKS, 0U);
            return Q_HANDLED();
        }
        
        case Q_EXIT_SIG: {
            // Cancel timeout
            QTimeEvt_disarm(&me->timeoutEvt);
            return Q_HANDLED();
        }
        
        case RESPONSE_SIG: {
            // Expected response received
            return Q_TRAN(&MyAO_processing);
        }
        
        case TIMEOUT_SIG: {
            // Timeout occurred - handle gracefully
            me->timeout_count++;
            
            if (me->timeout_count < MAX_RETRIES) {
                // Retry operation
                return Q_TRAN(&MyAO_retry);
            } else {
                // Give up and report error
                return Q_TRAN(&MyAO_error);
            }
        }
    }
    return Q_SUPER(&MyAO_active);
}
```

## Inter-AO Communication Patterns

### Publisher-Subscriber Pattern
```c
// Publisher AO
QState SensorAO_reading(SensorAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case ADC_COMPLETE_SIG: {
            // Create and publish sensor data event
            SensorDataEvt *sde = Q_NEW(SensorDataEvt, SENSOR_DATA_SIG);
            sde->sensor_id = me->sensor_id;
            sde->value = me->adc_reading;
            sde->timestamp = BSP_getTime();
            
            // Publish to all subscribers
            QACTIVE_PUBLISH(&sde->super, me);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&SensorAO_active);
}

// Subscriber AO initialization
void DisplayAO_ctor(void) {
    // Subscribe to sensor data
    QActive_subscribe(&AO_Display.super, SENSOR_DATA_SIG);
    QActive_subscribe(&AO_Display.super, ALARM_SIG);
}

// Subscriber event handling
QState DisplayAO_active(DisplayAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case SENSOR_DATA_SIG: {
            SensorDataEvt const *sde = Q_EVT_CAST(SensorDataEvt);
            
            // Update display with new sensor data
            update_display(sde->sensor_id, sde->value);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

### Command-Response Pattern
```c
// Command sender
QState ControllerAO_active(ControllerAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case USER_INPUT_SIG: {
            // Send command to actuator
            ActuatorCmdEvt *ace = Q_NEW(ActuatorCmdEvt, ACTUATOR_CMD_SIG);
            ace->command = MOVE_TO_POSITION;
            ace->position = me->target_position;
            ace->response_to = &me->super;  // Where to send response
            
            QACTIVE_POST(AO_Actuator, &ace->super, me);
            
            // Wait for response
            return Q_TRAN(&ControllerAO_waiting_response);
        }
    }
    return Q_SUPER(&QHsm_top);
}

// Command handler
QState ActuatorAO_active(ActuatorAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case ACTUATOR_CMD_SIG: {
            ActuatorCmdEvt const *ace = Q_EVT_CAST(ActuatorCmdEvt);
            
            // Execute command
            bool success = execute_command(ace->command, ace->position);
            
            // Send response back
            ActuatorRespEvt *are = Q_NEW(ActuatorRespEvt, ACTUATOR_RESP_SIG);
            are->success = success;
            are->actual_position = me->current_position;
            
            QACTIVE_POST(ace->response_to, &are->super, me);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

## Error Handling and Fault Tolerance

### Graceful Degradation Pattern
```c
// Hierarchical error handling
QState SystemAO_operational(SystemAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case FAULT_SIG: {
            FaultEvt const *fe = Q_EVT_CAST(FaultEvt);
            
            switch (fe->severity) {
                case FAULT_CRITICAL: {
                    // Critical fault - transition to safe state
                    return Q_TRAN(&SystemAO_safe_mode);
                }
                
                case FAULT_MAJOR: {
                    // Major fault - reduced functionality
                    return Q_TRAN(&SystemAO_degraded);
                }
                
                case FAULT_MINOR: {
                    // Minor fault - log and continue
                    log_fault(fe->fault_code);
                    return Q_HANDLED();
                }
            }
            break;
        }
    }
    return Q_SUPER(&QHsm_top);
}

QState SystemAO_degraded(SystemAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Disable non-essential functions
            disable_non_critical_systems();
            
            // Start recovery timer
            QTimeEvt_armX(&me->recoveryEvt, RECOVERY_TIMEOUT, 0U);
            
            return Q_HANDLED();
        }
        
        case RECOVERY_SIG: {
            // Attempt to return to normal operation
            if (system_health_check()) {
                return Q_TRAN(&SystemAO_operational);
            } else {
                // Still degraded - restart recovery timer
                QTimeEvt_armX(&me->recoveryEvt, RECOVERY_TIMEOUT, 0U);
                return Q_HANDLED();
            }
        }
    }
    return Q_SUPER(&SystemAO_operational); // Inherit fault handling
}
```

### Watchdog Pattern
```c
// Watchdog monitoring for Active Objects
typedef struct {
    QActive super;
    QTimeEvt watchdogEvt;
    uint32_t last_checkin[MAX_AOS];
    uint32_t checkin_count[MAX_AOS];
} WatchdogAO;

QState WatchdogAO_monitoring(WatchdogAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case WATCHDOG_CHECKIN_SIG: {
            CheckinEvt const *ce = Q_EVT_CAST(CheckinEvt);
            
            // Record checkin from Active Object
            me->last_checkin[ce->ao_id] = BSP_getTime();
            me->checkin_count[ce->ao_id]++;
            
            return Q_HANDLED();
        }
        
        case WATCHDOG_TIMEOUT_SIG: {
            // Check all Active Objects for timely checkins
            uint32_t current_time = BSP_getTime();
            
            for (uint8_t i = 0; i < MAX_AOS; i++) {
                if ((current_time - me->last_checkin[i]) > CHECKIN_TIMEOUT) {
                    // AO failed to check in - take action
                    FaultEvt *fe = Q_NEW(FaultEvt, FAULT_SIG);
                    fe->fault_code = FAULT_AO_TIMEOUT;
                    fe->severity = FAULT_MAJOR;
                    fe->ao_id = i;
                    
                    QACTIVE_PUBLISH(&fe->super, me);
                }
            }
            
            // Restart watchdog timer
            QTimeEvt_armX(&me->watchdogEvt, WATCHDOG_PERIOD, 0U);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

// Active Objects send periodic checkins
QState NormalAO_active(NormalAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case PERIODIC_SIG: {
            // Do normal work
            perform_normal_operation();
            
            // Send watchdog checkin
            CheckinEvt *ce = Q_NEW(CheckinEvt, WATCHDOG_CHECKIN_SIG);
            ce->ao_id = MY_AO_ID;
            QACTIVE_POST(AO_Watchdog, &ce->super, me);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

## Memory Management Patterns

### Static Memory Pattern
```c
// All memory allocated statically for deterministic behavior
#define SMALL_EVENT_POOL_SIZE  20U
#define MEDIUM_EVENT_POOL_SIZE 10U
#define LARGE_EVENT_POOL_SIZE  5U

// Event pool storage
static QF_MPOOL_EL(SmallEvt) l_smlPoolSto[SMALL_EVENT_POOL_SIZE];
static QF_MPOOL_EL(MediumEvt) l_medPoolSto[MEDIUM_EVENT_POOL_SIZE];
static QF_MPOOL_EL(LargeEvt) l_lrgPoolSto[LARGE_EVENT_POOL_SIZE];

// Active Object queue storage
static QEvt const *l_myAO_queueSto[QUEUE_DEPTH];

// Initialize memory pools
void memory_init(void) {
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF_poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));
    QF_poolInit(l_lrgPoolSto, sizeof(l_lrgPoolSto), sizeof(l_lrgPoolSto[0]));
}

// Monitor pool usage
void check_pool_usage(void) {
    uint32_t free_small = QF_getPoolMax(sizeof(SmallEvt)) - 
                         QF_getPoolMin(sizeof(SmallEvt));
    
    if (free_small < 5U) {
        // Pool getting low - take action
        log_warning("Small event pool low");
    }
}
```

### Event Recycling Pattern
```c
// Reuse events to minimize memory allocation
typedef struct {
    QEvt super;
    uint32_t data[10];  // Large data payload
    bool in_use;
} LargeDataEvt;

// Global pool of reusable events
static LargeDataEvt l_recyclePool[RECYCLE_POOL_SIZE];
static uint8_t l_recycleIndex = 0;

// Get recycled event
LargeDataEvt *get_recycled_event(void) {
    for (uint8_t i = 0; i < RECYCLE_POOL_SIZE; i++) {
        if (!l_recyclePool[i].in_use) {
            l_recyclePool[i].in_use = true;
            QEvt_ctor(&l_recyclePool[i].super, LARGE_DATA_SIG);
            return &l_recyclePool[i];
        }
    }
    return NULL; // Pool exhausted
}

// Return event to pool
void return_recycled_event(LargeDataEvt *evt) {
    evt->in_use = false;
    // Clear sensitive data if needed
    memset(evt->data, 0, sizeof(evt->data));
}

// Use in event handler
QState MyAO_handler(MyAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case LARGE_DATA_SIG: {
            LargeDataEvt *lde = (LargeDataEvt *)e;
            
            // Process the data
            process_large_data(lde->data);
            
            // Return to pool when done
            return_recycled_event(lde);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

## Performance Optimization

### Interrupt Priority Configuration
```c
// Configure interrupt priorities for QK
void configure_interrupts_qk(void) {
    // QK kernel awareness level
    #define QK_AWARE_ISR_PRIO  (QF_AWARE_ISR_CMSIS_PRI)
    
    // System tick - lowest priority among kernel-aware
    NVIC_SetPriority(SysTick_IRQn, QK_AWARE_ISR_PRIO + 1U);
    
    // High-priority time-critical interrupts
    NVIC_SetPriority(TIM1_UP_IRQn, QK_AWARE_ISR_PRIO + 0U);  // Motor control
    NVIC_SetPriority(ADC1_2_IRQn, QK_AWARE_ISR_PRIO + 0U);   // ADC sampling
    
    // Medium-priority communication interrupts  
    NVIC_SetPriority(USART1_IRQn, QK_AWARE_ISR_PRIO + 2U);   // Serial comm
    NVIC_SetPriority(CAN1_RX0_IRQn, QK_AWARE_ISR_PRIO + 2U); // CAN bus
    
    // Low-priority background interrupts
    NVIC_SetPriority(DMA1_Channel1_IRQn, QK_AWARE_ISR_PRIO + 3U);
    
    // Non-kernel-aware interrupts (emergency only)
    NVIC_SetPriority(WWDG_IRQn, 0U);  // Watchdog - highest priority
}
```

### Context Switch Optimization
```c
// Minimize work done in high-priority AOs
QState HighPrioAO_handler(HighPrioAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case CRITICAL_SIG: {
            // Do minimal processing in high-priority AO
            me->critical_data = extract_critical_info(e);
            
            // Delegate heavy processing to lower-priority AO
            ProcessEvt *pe = Q_NEW(ProcessEvt, PROCESS_SIG);
            pe->data = me->critical_data;
            QACTIVE_POST(AO_LowPrio, &pe->super, me);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

// Heavy processing in lower-priority AO
QState LowPrioAO_handler(LowPrioAO * const me, QEvt const * const e) {
    switch (e->sig) {
        case PROCESS_SIG: {
            ProcessEvt const *pe = Q_EVT_CAST(ProcessEvt);
            
            // Heavy processing can be preempted if needed
            complex_algorithm(pe->data);
            update_database(pe->data);
            generate_report(pe->data);
            
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
```

## Testing and Validation

### Timing Validation Pattern
```c
// Validate timing constraints at runtime
typedef struct {
    uint32_t start_time;
    uint32_t max_time;
    uint32_t total_time;
    uint32_t count;
    const char *name;
} TimingStats;

static TimingStats l_timingStats[MAX_AOS];

#define TIMING_START(ao_id) do { \
    l_timingStats[ao_id].start_time = BSP_getTime(); \
} while(0)

#define TIMING_END(ao_id) do { \
    uint32_t elapsed = BSP_getTime() - l_timingStats[ao_id].start_time; \
    l_timingStats[ao_id].total_time += elapsed; \
    l_timingStats[ao_id].count++; \
    if (elapsed > l_timingStats[ao_id].max_time) { \
        l_timingStats[ao_id].max_time = elapsed; \
    } \
    if (elapsed > MAX_RTC_TIME_US) { \
        Q_ERROR(); /* RTC constraint violated */ \
    } \
} while(0)

// Use in event handlers
QState MyAO_handler(MyAO * const me, QEvt const * const e) {
    TIMING_START(MY_AO_ID);
    
    QState result = Q_SUPER(&QHsm_top);
    switch (e->sig) {
        case MY_SIG: {
            // Event processing
            result = Q_HANDLED();
            break;
        }
    }
    
    TIMING_END(MY_AO_ID);
    return result;
}
```

These patterns provide a comprehensive foundation for developing robust, real-time embedded systems using the QK kernel. They emphasize the key principles of preemptive scheduling, non-blocking execution, and deterministic behavior that make QK suitable for time-critical applications.