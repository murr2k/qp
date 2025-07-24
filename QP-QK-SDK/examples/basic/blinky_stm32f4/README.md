# Blinky Example for STM32F4 with QP-QK

This is a basic example demonstrating the use of the Quantum Platform (QP) framework with the QK preemptive kernel on an STM32F4 microcontroller. The example implements a simple LED blinking application using the Active Object pattern.

## Overview

The Blinky example showcases:
- Single Active Object implementation
- Periodic timer events
- Basic state machine with two states
- QK kernel integration
- QS software tracing
- Board Support Package (BSP) usage

## Hardware Requirements

- STM32F4 Discovery board or Nucleo-F411RE
- ST-Link programmer/debugger
- USB cable for power and programming

## Software Requirements

- ARM GCC toolchain (`arm-none-eabi-gcc`)
- ST-Link tools (`st-flash`, `st-info`)
- QP framework (included in SDK)
- QSpy for trace visualization (optional)

## Project Structure

```
blinky_stm32f4/
├── src/
│   ├── main.c              # Main application file
│   ├── bsp.c               # Board Support Package
│   └── blinky.c            # Blinky Active Object implementation
├── inc/
│   ├── blinky.h            # Blinky Active Object header
│   └── project_config.h    # Project configuration
├── build/                  # Build output directory
├── Makefile               # Build configuration
├── build_config.yaml      # SDK build configuration
├── flash_config.json      # Flash configuration
└── README.md              # This file
```

## Building the Project

1. **Using the QP-QK SDK build tool:**
   ```bash
   python3 ../../tools/builders/build.py --project . --config release
   ```

2. **Using Make directly:**
   ```bash
   make clean
   make
   ```

The build process will generate:
- `build/blinky.elf` - Executable with debug information
- `build/blinky.bin` - Binary file for flashing
- `build/blinky.hex` - Intel HEX format
- `build/blinky.asm` - Disassembly listing

## Flashing the Firmware

1. **Using the QP-QK SDK flash tool:**
   ```bash
   python3 ../../tools/deployers/flash.py --project . --interface stlink
   ```

2. **Using ST-Link tools directly:**
   ```bash
   st-flash write build/blinky.bin 0x08000000
   ```

## Expected Behavior

After flashing the firmware:

1. **LED LD2 (Green)** will blink at 1 Hz (500ms on, 500ms off)
2. **LED LD3 (Orange)** will provide a heartbeat indication during idle time
3. **User button** will toggle LED LD4 (Red) when pressed
4. **LED LD5 (Blue)** will indicate any error conditions

## Code Structure

### Active Object Design

The Blinky Active Object implements a simple two-state machine:

```
    ┌─────────────┐
    │    off      │ ◄──┐
    └─────────────┘    │
           │ TIMEOUT_SIG
           ▼            │
    ┌─────────────┐     │
    │     on      │ ────┘
    └─────────────┘ TIMEOUT_SIG
```

### State Machine Implementation

```c
// Blinky Active Object states
QState Blinky_initial(Blinky * const me, QEvt const * const e);
QState Blinky_off(Blinky * const me, QEvt const * const e);
QState Blinky_on(Blinky * const me, QEvt const * const e);
```

### Event Signals

```c
enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG,   // Timer timeout
    BUTTON_SIG,                 // Button press
    MAX_SIG
};
```

## QK Kernel Features Demonstrated

1. **Preemptive Scheduling**: Button events can interrupt LED timing
2. **Priority-Based Execution**: Different priority levels for different tasks
3. **Non-Blocking Execution**: All event handlers complete quickly
4. **Single Stack Architecture**: Efficient memory usage
5. **Interrupt Integration**: Proper QK-aware interrupt handling

## QS Software Tracing

The example includes QS tracing for debugging and analysis:

1. **Connect QSpy:**
   ```bash
   qspy -c COM3 -b 115200  # Windows
   qspy -c /dev/ttyACM0 -b 115200  # Linux
   ```

2. **Trace Output:**
   - State transitions
   - Event dispatching
   - Timer operations
   - Button presses
   - System performance metrics

## Memory Usage

Typical memory usage for this example:

| Resource | Usage | Available | Percentage |
|----------|-------|-----------|------------|
| Flash    | ~8KB  | 512KB     | 1.5%       |
| RAM      | ~2KB  | 128KB     | 1.5%       |
| Stack    | ~1KB  | -         | -          |

## Performance Metrics

- **Response Time**: < 1ms for button press
- **Jitter**: < 10μs for periodic events
- **CPU Utilization**: < 1% during normal operation
- **Context Switch Time**: < 5μs

## Configuration Options

Edit `project_config.h` to customize:

```c
// LED blink period (in system ticks)
#define BLINK_PERIOD_TICKS      (BSP_TICKS_PER_SEC / 2U)

// Active Object priority
#define AO_BLINKY_PRIO          3U

// Event queue depth
#define BLINKY_QUEUE_LEN        10U

// Enable/disable QS tracing
#define Q_SPY                   1
```

## Troubleshooting

### Build Issues

1. **Toolchain not found:**
   - Ensure ARM GCC toolchain is installed and in PATH
   - Check toolchain version compatibility

2. **QP framework not found:**
   - Verify QP path in build configuration
   - Check QP framework installation

### Flash Issues

1. **ST-Link not detected:**
   - Check USB connection
   - Install ST-Link drivers
   - Verify board power

2. **Flash write failed:**
   - Check target connection
   - Verify memory map settings
   - Try erasing flash first

### Runtime Issues

1. **LEDs not blinking:**
   - Check GPIO configuration
   - Verify system clock setup
   - Use QSpy to debug state machine

2. **Button not responding:**
   - Check interrupt configuration
   - Verify button wiring/pull-ups
   - Check QK interrupt priorities

## Extending the Example

### Adding More LEDs

1. Update `bsp.c` to configure additional GPIO pins
2. Add new LED control functions
3. Modify state machine to control multiple LEDs

### Adding Serial Communication

1. Configure UART in BSP
2. Create communication Active Object
3. Implement message processing

### Adding Sensors

1. Configure ADC/I2C peripherals
2. Create sensor reader Active Object
3. Implement data processing pipeline

## Learning Objectives

This example teaches:

1. **QP Framework Basics**: Active Objects, events, state machines
2. **QK Kernel Usage**: Preemptive scheduling, priorities, timing
3. **Embedded Best Practices**: Hardware abstraction, error handling
4. **Real-Time Programming**: Deterministic behavior, bounded execution
5. **Debugging Techniques**: QS tracing, performance monitoring

## Next Steps

After understanding this example:

1. Study the **DPP (Dining Philosophers)** example for inter-AO communication
2. Explore the **Sensor Network** example for multiple Active Objects
3. Learn about **QM Modeling Tool** for visual state machine design
4. Practice **QUTest framework** for automated testing

## References

- [QP Framework Documentation](https://www.state-machine.com/qp)
- [QK Kernel Guide](https://www.state-machine.com/qp/qk.html)
- [STM32F4 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020.pdf)
- [QS Software Tracing](https://www.state-machine.com/qp/qs.html)

## License

This example is part of the QP-QK SDK and follows the same licensing terms as the QP framework (GPL/Commercial dual license).