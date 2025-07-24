# Quantum Leaps (QP) Microcontroller Development Reference Guide

## Overview

This reference guide provides essential information for developing microcontroller firmware using the Quantum Platform (QP) framework by Miro Samek. QP implements an event-driven Active Object pattern with hierarchical state machines, offering a modern alternative to traditional RTOS-based development.

## Core Concepts Quick Reference

### Active Objects
- **Definition**: Event-driven, encapsulated objects with private event queues
- **Communication**: Asynchronous message passing only
- **Execution**: Run-to-Completion (RTC) semantics
- **Benefits**: No shared state, no blocking, no race conditions

### Hierarchical State Machines (HSMs)
- **States**: Can contain nested substates
- **Behavioral Inheritance**: Substates inherit from parent states
- **Transitions**: Regular, initial, internal, and self-transitions
- **Entry/Exit Actions**: Guaranteed execution on state changes
- **Guards**: Boolean conditions on transitions

### Event-Driven Architecture
- **Events**: Typed messages with optional parameters
- **Event Pools**: Pre-allocated memory for dynamic events
- **Publishing**: One-to-many event distribution
- **Direct Posting**: Point-to-point event delivery

## QP Framework Components

### Core Modules
1. **QEP** - Quantum Event Processor (hierarchical state machines)
2. **QF** - Quantum Framework (active object runtime)
3. **QV/QK/QXK** - Built-in kernels
4. **QS** - Quantum Spy (software tracing)
5. **QM** - QP Modeler (visual modeling tool)

### Kernel Selection Guide

| Kernel | Type | Use When | Avoid When |
|--------|------|----------|------------|
| **QV** | Cooperative | - Simple systems<br>- All events process quickly<br>- Safety-critical (simplest) | Need preemption |
| **QK** | Preemptive non-blocking | - Hard real-time<br>- Mixed priorities<br>- Safety-critical | Need blocking operations |
| **QXK** | Dual-mode | - Third-party middleware<br>- Legacy code integration | Safety-critical systems |

## Platform-Specific Implementation

### ARM Cortex-M
```c
// Typical main.c structure
#include "qpc.h"
#include "bsp.h"
#include "app.h"

int main(void) {
    QF_init();       // Initialize framework
    BSP_init();      // Initialize board support
    
    // Initialize event pools
    static QF_MPOOL_EL(SmallEvent) smlPoolSto[10];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(SmallEvent));
    
    // Initialize publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive_psInit(subscrSto, Q_DIM(subscrSto));
    
    // Start active objects
    static QEvt const *tableQueueSto[5];
    static StackType_t tableStackSto[256];
    Table_ctor();
    QACTIVE_START(AO_Table,
                  1U,                          // priority
                  tableQueueSto, Q_DIM(tableQueueSto),
                  tableStackSto, sizeof(tableStackSto),
                  (void *)0);
                  
    return QF_run(); // Run the framework
}
```

### Memory Configuration
```c
// Typical memory requirements per Active Object
// QV kernel: ~100-200 bytes
// QK kernel: ~150-300 bytes  
// QXK kernel: ~1KB+ (includes stack)

// Event pool sizing
#define SMALL_EVENT_SIZE  sizeof(QEvt)
#define MEDIUM_EVENT_SIZE sizeof(MyEventMedium)
#define LARGE_EVENT_SIZE  sizeof(MyEventLarge)

// Queue depth calculation
#define QUEUE_DEPTH(max_events, safety_margin) \
    ((max_events) + (safety_margin))
```

### Interrupt Integration
```c
// ISR posting events to Active Objects
void TIMER0_IRQHandler(void) {
    QK_ISR_ENTRY();  // Inform QK kernel
    
    // Clear interrupt flag
    TIMER0->INTCLR = 0x1U;
    
    // Post time event
    static QEvt const tickEvt = QEVT_INITIALIZER(TICK_SIG);
    QACTIVE_POST(AO_Clock, &tickEvt, &l_TIMER0_IRQHandler);
    
    QK_ISR_EXIT();   // Inform QK kernel
}
```

## State Machine Patterns

### Basic State Pattern
```c
QState MyAO_stateA(MyAO * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // Entry actions
            BSP_ledOn();
            status_ = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            // Exit actions
            BSP_ledOff();
            status_ = Q_HANDLED();
            break;
        }
        case MY_EVENT_SIG: {
            // Handle event and transition
            me->counter++;
            status_ = Q_TRAN(&MyAO_stateB);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
```

### Hierarchical State Pattern
```c
// Parent state
QState MyAO_operational(MyAO * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case FAULT_SIG: {
            status_ = Q_TRAN(&MyAO_fault);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

// Child state inherits FAULT_SIG handling
QState MyAO_running(MyAO * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        case PAUSE_SIG: {
            status_ = Q_TRAN(&MyAO_paused);
            break;
        }
        default: {
            status_ = Q_SUPER(&MyAO_operational);
            break;
        }
    }
    return status_;
}
```

## Best Practices

### Event Design
1. **Immutable Events**: Never modify published events
2. **Event Pools**: Use appropriate pool sizes
3. **Signal Ranges**: Reserve ranges for different subsystems
4. **Parameter Events**: Derive from base QEvt

### Active Object Design
1. **Single Responsibility**: One clear purpose per AO
2. **Minimal Interfaces**: Communicate via events only
3. **Priority Assignment**: Use Rate Monotonic Analysis
4. **Queue Sizing**: Monitor high-water marks

### State Machine Design
1. **State Granularity**: Not too many, not too few
2. **Orthogonal Regions**: For independent behaviors
3. **History States**: For resuming interrupted behavior
4. **Guard Conditions**: Keep simple and fast

## Testing Strategies

### Unit Testing with QUTest
```python
# test_myao.py
import sys
import os
from qspypy.qspy import *

# Test fixture
def setup():
    expect("@timestamp BSP_INIT")
    expect("@timestamp AO_MyAO Init")
    glb_filter(GRP_ALL)
    current_obj(OBJ_AO, "AO_MyAO")

# Test case
def test_myao_initial_transition():
    post("MY_START_SIG")
    expect("@timestamp Obj=AO_MyAO,Sig=MY_START_SIG")
    expect("@timestamp BSP Led ON")
    expect("@timestamp ===> Tran Obj=AO_MyAO,Sig=MY_START_SIG,State=MyAO_idle->MyAO_active")
```

### Integration Testing
- Use QS/QSpy for system-level tracing
- Monitor event flow between Active Objects
- Verify timing constraints
- Check resource usage (CPU, memory)

## Common Pitfalls and Solutions

| Pitfall | Solution |
|---------|----------|
| Blocking in Active Objects | Use only non-blocking operations |
| Shared state between AOs | Communicate via events only |
| Event pool exhaustion | Size pools appropriately, monitor usage |
| Priority inversion | Use proper priority assignment |
| Stack overflow (QXK) | Increase stack sizes, profile usage |
| Missing state transitions | Use QM tool for visual verification |

## Performance Optimization

### Memory Optimization
- Use QMsm for smaller code size
- Minimize event parameter sizes
- Share common event types
- Use static allocation

### CPU Optimization
- Keep RTC steps short
- Use QK for better responsiveness
- Minimize state nesting depth
- Profile with QS timestamps

### Power Optimization
- Leverage idle callback
- Use tickless operation
- Minimize active object count
- Batch event processing

## Debugging Techniques

### QS/QSpy Setup
```c
// Enable QS in target
#define Q_SPY
#include "qpc.h"

// Initialize QS
uint8_t qsTxBuf[1024];
uint8_t qsRxBuf[128];
QS_INIT(qsTxBuf, sizeof(qsTxBuf),
        qsRxBuf, sizeof(qsRxBuf));

// Filter setup
QS_GLB_FILTER(QS_ALL_RECORDS);
QS_LOC_FILTER(QS_AO_OBJ, AO_MyAO);
```

### Common Debug Scenarios
1. **State machine issues**: Enable QS_SM records
2. **Event flow problems**: Enable QS_QF records  
3. **Timing issues**: Use QS timestamps
4. **Memory leaks**: Monitor pool usage

## Resources and Learning Path

### Getting Started
1. Read "Practical UML Statecharts in C/C++" book
2. Complete Blinky example
3. Study DPP example
4. Build custom project

### Intermediate
1. Master QM modeling tool
2. Implement complex HSMs
3. Integrate QS/QSpy
4. Learn QUTest framework

### Advanced
1. Custom BSP development
2. RTOS integration
3. Safety-critical design
4. Performance optimization

### Community Resources
- [State-machine.com](https://www.state-machine.com)
- [Quantum Leaps Blog](https://www.state-machine.com/category/blog)
- [YouTube Course](https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM)
- [GitHub Examples](https://github.com/QuantumLeaps)

## Quick Command Reference

### Building Projects
```bash
# QM code generation
qm model.qm -c

# GNU ARM toolchain
make GNU_ARM

# IAR toolchain  
make IAR

# Keil/ARM toolchain
make ARM
```

### QSpy Usage
```bash
# Serial connection
qspy -c COM1 -b 115200

# TCP/IP connection
qspy -t

# Save trace to file
qspy -c COM1 -f trace.txt

# With timing info
qspy -c COM1 -T
```

This reference guide provides the essential information needed to develop professional microcontroller firmware using the Quantum Leaps framework. The event-driven Active Object pattern combined with hierarchical state machines offers a powerful, modern approach to embedded software development that scales from simple 8-bit MCUs to complex 32-bit systems.