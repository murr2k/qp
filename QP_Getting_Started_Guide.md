# Getting Started with QP (Quantum Platform) for Microcontroller Firmware Development

## Table of Contents
1. [Introduction](#introduction)
2. [Step-by-Step Getting Started Process](#step-by-step-getting-started-process)
3. [Choosing Between QP/C and QP/C++](#choosing-between-qpc-and-qpc)
4. [Selecting the Right Kernel](#selecting-the-right-kernel)
5. [Basic Project Structure](#basic-project-structure)
6. [Common Patterns and Best Practices](#common-patterns-and-best-practices)
7. [Integration with Popular Microcontroller Platforms](#integration-with-popular-microcontroller-platforms)
8. [Resources for Learning More](#resources-for-learning-more)

## Introduction

QP (Quantum Platform) is a family of lightweight, open-source software frameworks for building responsive and modular real-time embedded applications based on hierarchical state machines (UML statecharts) and the active object design pattern. QP enables event-driven programming without blocking, making it ideal for resource-constrained microcontrollers.

### Key Benefits:
- **Event-driven architecture** - No blocking calls, better responsiveness
- **Active Object pattern** - Natural concurrency without traditional RTOS complexities
- **Hierarchical State Machines** - Manage complex behavior elegantly
- **Minimal resource usage** - Suitable for small microcontrollers
- **Deterministic behavior** - Predictable real-time performance
- **Built-in tracing** - QS (Q-Spy) software tracing for debugging

## Step-by-Step Getting Started Process

### Step 1: Download and Install QP
1. Visit [state-machine.com](https://www.state-machine.com/products/qp)
2. Download the appropriate QP version (QP/C or QP/C++)
3. Extract the archive to your development directory (e.g., `C:\qp\` on Windows)

### Step 2: Install Development Tools
1. **QM (QP Modeler)** - Free graphical modeling tool
   - Download from [state-machine.com/qm](https://www.state-machine.com/qm)
   - Install and register (free for QP GPL/evaluation)

2. **Compiler/IDE** for your target:
   - ARM: Keil MDK, IAR EWARM, or GNU ARM Embedded
   - AVR: Atmel Studio or avr-gcc
   - MSP430: Code Composer Studio or msp430-gcc

3. **QTools** (optional but recommended)
   - QSPY host application for software tracing
   - QUTest unit testing framework
   - QClean for code metrics

### Step 3: Run Example Projects
1. Navigate to `qpc/examples/` or `qpcpp/examples/`
2. Choose an example for your target platform
3. Open the project in your IDE
4. Build and flash to your target
5. Observe the behavior (typically blinking LEDs or similar)

### Step 4: Study the Documentation
1. Read "Getting Started with QP" guide in `qpc/doc/`
2. Review the "QP/C Reference Manual"
3. Study the example code structure
4. Watch video tutorials on state-machine.com

### Step 5: Create Your First Project
1. Copy an example project as a template
2. Modify the BSP (Board Support Package) for your hardware
3. Design your state machines using QM
4. Generate code and integrate with your project
5. Build, test, and iterate

## Choosing Between QP/C and QP/C++

### QP/C - Choose when:
- **Target has limited resources** (< 32KB ROM, < 4KB RAM)
- **C-only toolchain** is required
- **Team expertise** is primarily in C
- **Legacy code** integration requires C
- **Certification** requirements favor C (easier to verify)

### QP/C++ - Choose when:
- **Resources are adequate** (> 64KB ROM, > 8KB RAM)
- **C++ features** would benefit the design (templates, namespaces)
- **Team prefers** object-oriented programming
- **Code reuse** through inheritance is important
- **Modern C++** features (C++11/14) are desired

### Feature Comparison:
| Feature | QP/C | QP/C++ |
|---------|------|--------|
| Minimum ROM | ~4KB | ~8KB |
| Minimum RAM | ~1KB | ~2KB |
| State machine syntax | Function pointers | Member functions |
| Encapsulation | Struct + naming | Classes + access control |
| Code generation | C structures | C++ classes |
| Type safety | Basic | Strong |

## Selecting the Right Kernel

QP offers three real-time kernels, each optimized for different use cases:

### QV (Cooperative Kernel)
**Best for:**
- Simple applications with few active objects
- Systems where all processing fits in ISR time
- Ultra-low RAM usage (no per-task stacks)
- Predictable, run-to-completion semantics

**Characteristics:**
- Non-preemptive
- Single stack
- Lowest overhead
- Priority-based scheduling

### QK (Preemptive Non-Blocking Kernel)
**Best for:**
- Systems requiring preemption
- Mixed priority tasks
- Better real-time response than QV
- Still maintaining single-stack efficiency

**Characteristics:**
- Preemptive
- Single stack with stack-sharing
- Priority ceiling protocol
- Efficient context switching

### QXK (Dual-Mode Kernel)
**Best for:**
- Complex systems needing both models
- Integration with blocking code
- Gradual migration from RTOS
- Maximum flexibility

**Characteristics:**
- Supports both active objects and threads
- Multiple stacks (one per thread)
- Can call blocking functions
- Compatible with existing RTOS code

### Kernel Selection Guide:
```
Start -> Need blocking calls? 
         |-- Yes -> QXK
         |-- No --> Need preemption?
                    |-- No --> QV
                    |-- Yes -> Need extended threads?
                              |-- No --> QK
                              |-- Yes -> QXK
```

## Basic Project Structure

A typical QP project follows this structure:

```
project/
├── src/
│   ├── main.c              # Application entry point
│   ├── bsp.c               # Board Support Package
│   ├── active_objects/     # Active object implementations
│   │   ├── blinky.c/h      # Example active object
│   │   └── button.c/h      # Another active object
│   └── qm_models/          # QM model files
│       └── model.qm        # Graphical state machine models
├── include/
│   ├── bsp.h               # BSP interface
│   ├── signals.h           # System-wide signal definitions
│   └── active_objects.h    # Active object interfaces
├── qp/                     # QP framework source (or link to it)
├── third_party/            # External libraries
└── build/                  # Build outputs

```

### Key Components:

1. **main.c** - System initialization:
```c
int main(void) {
    QF_init();       // Initialize framework
    BSP_init();      // Initialize hardware
    
    // Initialize active objects
    Blinky_ctor();
    Button_ctor();
    
    // Start active objects
    QACTIVE_START(AO_Blinky, /*priority*/1U,
                  blinkyQueueSto, Q_DIM(blinkyQueueSto),
                  /*stack*/NULL, /*stack size*/0U,
                  /*init event*/NULL);
    
    return QF_run(); // Run the framework
}
```

2. **bsp.c** - Hardware abstraction:
```c
void BSP_init(void) {
    // Initialize clocks, GPIO, peripherals
    SystemClock_Config();
    GPIO_Init();
    
    // Initialize QP tick timer
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
}

void QF_onStartup(void) {
    // Set interrupt priorities for QP
}
```

3. **Active Object** - Basic structure:
```c
typedef struct {
    QActive super;      // Inherit from QActive
    QTimeEvt timeEvt;   // Private time event
    uint8_t count;      // Private data
} Blinky;

static Blinky l_blinky; // Singleton instance

void Blinky_ctor(void) {
    QActive_ctor(&l_blinky.super, Q_STATE_CAST(&Blinky_initial));
    QTimeEvt_ctorX(&l_blinky.timeEvt, &l_blinky.super,
                   TIMEOUT_SIG, 0U);
}
```

## Common Patterns and Best Practices

### 1. Event-Driven Design
- **Never block** in state machine code
- Use time events instead of delays
- Post events asynchronously
- Design for run-to-completion

### 2. State Machine Patterns

**Pattern: Deferred Event**
```c
// Defer an event in one state
case COMPLEX_REQUEST_SIG:
    status_ = Q_DEFER(&me->deferred);
    break;

// Recall in another state
case Q_ENTRY_SIG:
    status_ = Q_RECALL(&me->deferred);
    break;
```

**Pattern: Orthogonal Regions**
```c
// Parent state with two regions
QState Parent_handler(Parent *me, QEvt const *e) {
    switch (e->sig) {
    case Q_INIT_SIG:
        // Initialize both regions
        QActive_post(&me->region1, Q_NEW(QEvt, START_SIG));
        QActive_post(&me->region2, Q_NEW(QEvt, START_SIG));
        break;
    }
}
```

### 3. Memory Management
- Use static allocation where possible
- Implement memory pools for dynamic events
- Size event queues appropriately
- Monitor queue usage with QS

### 4. Event Design
```c
// Good: Specific, meaningful events
enum SystemSignals {
    BUTTON_PRESSED_SIG = Q_USER_SIG,
    BUTTON_RELEASED_SIG,
    TEMPERATURE_READY_SIG,
    MOTOR_STALLED_SIG,
    // ...
};

// Good: Event with parameters
typedef struct {
    QEvt super;
    uint16_t temperature;
    uint8_t sensor_id;
} TempReadyEvt;
```

### 5. Active Object Communication
- Use publish-subscribe for loose coupling
- Direct posting for tight coupling
- Avoid shared data; pass by events
- Use QF critical sections sparingly

### 6. Testing Strategies
- Unit test state machines with QUTest
- Use QS (Q-Spy) for runtime tracing
- Mock hardware in BSP for testing
- Test event sequences systematically

## Integration with Popular Microcontroller Platforms

### ARM Cortex-M (STM32, NXP, etc.)
```c
// Typical STM32 integration
void SysTick_Handler(void) {
    QXK_ISR_ENTRY();    // Inform QP about ISR entry
    QF_TICK_X(0U, &l_SysTick);
    QXK_ISR_EXIT();     // Inform QP about ISR exit
}

void EXTI0_IRQHandler(void) {
    QXK_ISR_ENTRY();
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR = EXTI_PR_PR0;  // Clear flag
        QACTIVE_POST(AO_Button, Q_NEW(QEvt, BUTTON_PRESSED_SIG), 0U);
    }
    QXK_ISR_EXIT();
}
```

### Arduino (AVR/ARM)
- Use Arduino-QP library
- Simplified BSP for Arduino API
- Examples for common shields
- Compatible with Arduino IDE

### ESP32/ESP8266
- Integrate with ESP-IDF
- Use FreeRTOS compatibility layer
- Support for WiFi/BLE events
- Power management integration

### MSP430 (Ultra-Low Power)
```c
// Low-power idle callback
void QV_onIdle(void) {
    QF_INT_DISABLE();
    __low_power_mode_3();  // Enter LPM3
    QF_INT_ENABLE();
}
```

### PIC32
- MPLAB Harmony integration
- DMA event generation
- Peripheral event system

## Resources for Learning More

### Official Resources
1. **Books:**
   - "Practical UML Statecharts in C/C++" by Miro Samek
   - "Practical Statecharts in C/C++" (older but still valuable)

2. **Online Documentation:**
   - [QP Documentation](https://www.state-machine.com/doc)
   - [QP/C Reference Manual](https://www.state-machine.com/qpc)
   - [QP/C++ Reference Manual](https://www.state-machine.com/qpcpp)

3. **Video Courses:**
   - [Modern Embedded Systems Programming](https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM)
   - [State Machines & Active Objects](https://www.state-machine.com/course)

### Community Resources
1. **Forums and Support:**
   - [SourceForge QP Forum](https://sourceforge.net/p/qpc/discussion/)
   - [Reddit r/embedded](https://www.reddit.com/r/embedded/)
   - Stack Overflow tag: `qp-framework`

2. **Example Projects:**
   - GitHub: Search "QP framework"
   - Official examples in QP distribution
   - Community contributions

3. **Tools and Utilities:**
   - QM (QP Modeler) - Free modeling tool
   - QView - Real-time visualization
   - QSpyView - Trace analysis
   - QUTest - Unit testing framework

### Learning Path Recommendations

1. **Beginner (Week 1-2):**
   - Read "Getting Started" guide
   - Run and modify Blinky example
   - Learn basic QM modeling
   - Understand event-driven concepts

2. **Intermediate (Week 3-4):**
   - Study DPP (Dining Philosophers) example
   - Implement custom active objects
   - Learn hierarchical state machines
   - Practice with QS tracing

3. **Advanced (Month 2+):**
   - Design complex state machines
   - Optimize for your platform
   - Implement custom BSPs
   - Contribute to community

### Best Practices Summary

1. **Start simple** - Blinky first, then add complexity
2. **Model visually** - Use QM for state machines
3. **Think events** - Not sequential flow
4. **Test early** - Use QS tracing from day one
5. **Read examples** - Learn from provided code
6. **Ask questions** - Active community support

## Conclusion

QP provides a modern, efficient approach to embedded firmware development that scales from simple 8-bit MCUs to complex 32-bit systems. By following this guide and leveraging the active object pattern with hierarchical state machines, you can build more maintainable, responsive, and robust embedded systems.

Remember: The key to success with QP is embracing the event-driven paradigm. Think in terms of events and state machines rather than sequential code, and you'll unlock the full power of the framework.

Happy coding with QP!