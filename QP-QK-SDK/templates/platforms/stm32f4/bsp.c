/**
 * @file bsp.c
 * @brief Board Support Package for STM32F4 QK Projects
 * @version 1.0.0
 * @date 2025-01-24
 * 
 * Template BSP implementation for STM32F4 platforms using QP/C with QK kernel
 * This file provides hardware abstraction and platform-specific functionality
 * for AI-generated microcontroller firmware projects.
 */

#include "project_template.h"
#include "stm32f4xx_hal.h"

Q_DEFINE_THIS_FILE

//============================================================================
// LOCAL CONSTANTS AND MACROS
//============================================================================

// LED GPIO configuration
#define LED_COUNT               4U
#define LED1_PIN                GPIO_PIN_12
#define LED2_PIN                GPIO_PIN_13
#define LED3_PIN                GPIO_PIN_14
#define LED4_PIN                GPIO_PIN_15
#define LED_GPIO_PORT           GPIOD
#define LED_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()

// Button configuration
#define BUTTON_PIN              GPIO_PIN_0
#define BUTTON_GPIO_PORT        GPIOA
#define BUTTON_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUTTON_EXTI_IRQn        EXTI0_IRQn

// UART configuration for QS trace output
#define QS_UART                 USART2
#define QS_UART_CLK_ENABLE()    __HAL_RCC_USART2_CLK_ENABLE()
#define QS_UART_GPIO_PORT       GPIOA
#define QS_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define QS_UART_TX_PIN          GPIO_PIN_2
#define QS_UART_RX_PIN          GPIO_PIN_3
#define QS_UART_AF              GPIO_AF7_USART2
#define QS_UART_BAUDRATE        115200U

// System timing
#define BSP_SYSTICK_FREQ        1000U   // 1ms system tick

//============================================================================
// LOCAL VARIABLES
//============================================================================

// System tick counter for timing
static volatile uint32_t l_tickCtr = 0U;

// UART handle for QS tracing
#ifdef Q_SPY
static UART_HandleTypeDef l_uartHandle;
#endif

// Random number seed
static uint32_t l_rndSeed;

//============================================================================
// LOCAL FUNCTION PROTOTYPES
//============================================================================

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Init(void);
static void Error_Handler(void);

//============================================================================
// BSP IMPLEMENTATION
//============================================================================

void BSP_init(void) {
    // Initialize HAL library
    if (HAL_Init() != HAL_OK) {
        Error_Handler();
    }
    
    // Configure system clock
    SystemClock_Config();
    
    // Initialize GPIO
    GPIO_Init();
    
    // Initialize UART for QS tracing
    UART_Init();
    
    // Initialize random number seed
    l_rndSeed = 0x12345678U;
    
    // Initialize LEDs (all off)
    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        BSP_ledOff(i);
    }
    
    // QS trace initialization
#ifdef Q_SPY
    if (!QS_INIT((void *)0)) {
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_tickCtr);
    QS_USR_DICTIONARY(QS_SENSOR_DATA);
    QS_USR_DICTIONARY(QS_GPIO_CHANGE);
    QS_USR_DICTIONARY(QS_TIMING_INFO);
    QS_USR_DICTIONARY(QS_ERROR_INFO);
#endif
    
    // BSP initialization complete trace
    QS_BEGIN_ID(QS_USER_00, 0U)
        QS_STR_("BSP_init");
    QS_END_()
}

void BSP_terminate(int16_t result) {
    (void)result;
    
    // Turn on error LED
    BSP_ledOn(3);
    
    // Disable all interrupts
    __disable_irq();
    
    // Infinite loop
    for (;;) {
        __NOP();
    }
}

//============================================================================
// LED CONTROL FUNCTIONS
//============================================================================

void BSP_ledOn(uint8_t led) {
    if (led < LED_COUNT) {
        GPIO_TypeDef *port = LED_GPIO_PORT;
        uint16_t pin = 0;
        
        switch (led) {
            case 0: pin = LED1_PIN; break;
            case 1: pin = LED2_PIN; break;
            case 2: pin = LED3_PIN; break;
            case 3: pin = LED4_PIN; break;
            default: return;
        }
        
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        
        // QS trace
        QS_BEGIN_ID(QS_GPIO_CHANGE, 0U)
            QS_U8_(led);
            QS_U8_(1U);  // On
        QS_END_()
    }
}

void BSP_ledOff(uint8_t led) {
    if (led < LED_COUNT) {
        GPIO_TypeDef *port = LED_GPIO_PORT;
        uint16_t pin = 0;
        
        switch (led) {
            case 0: pin = LED1_PIN; break;
            case 1: pin = LED2_PIN; break;
            case 2: pin = LED3_PIN; break;
            case 3: pin = LED4_PIN; break;
            default: return;
        }
        
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        
        // QS trace
        QS_BEGIN_ID(QS_GPIO_CHANGE, 0U)
            QS_U8_(led);
            QS_U8_(0U);  // Off
        QS_END_()
    }
}

void BSP_ledToggle(uint8_t led) {
    if (led < LED_COUNT) {
        GPIO_TypeDef *port = LED_GPIO_PORT;
        uint16_t pin = 0;
        
        switch (led) {
            case 0: pin = LED1_PIN; break;
            case 1: pin = LED2_PIN; break;
            case 2: pin = LED3_PIN; break;
            case 3: pin = LED4_PIN; break;
            default: return;
        }
        
        HAL_GPIO_TogglePin(port, pin);
        
        // Read current state for tracing
        GPIO_PinState state = HAL_GPIO_ReadPin(port, pin);
        
        // QS trace
        QS_BEGIN_ID(QS_GPIO_CHANGE, 0U)
            QS_U8_(led);
            QS_U8_((state == GPIO_PIN_SET) ? 1U : 0U);
        QS_END_()
    }
}

//============================================================================
// SYSTEM TIMING FUNCTIONS
//============================================================================

void BSP_tickHook(void) {
    // Increment system tick counter
    ++l_tickCtr;
    
    // Update random seed
    l_rndSeed = l_rndSeed * 1103515245U + 12345U;
    
    // Periodic BSP processing (every 100ms)
    if ((l_tickCtr % 100U) == 0U) {
        // Example: watchdog refresh, periodic checks, etc.
        // {{PERIODIC_BSP_PROCESSING}}
    }
}

uint32_t BSP_getTime(void) {
    return l_tickCtr;
}

uint32_t BSP_getTimeUs(void) {
    // Get current SysTick value (counts down)
    uint32_t systick_val = SysTick->VAL;
    uint32_t systick_load = SysTick->LOAD;
    
    // Convert to microseconds
    uint32_t ticks_us = (systick_load + 1U - systick_val) * 1000U / (systick_load + 1U);
    
    return (l_tickCtr * 1000U) + ticks_us;
}

//============================================================================
// RANDOM NUMBER GENERATION
//============================================================================

uint32_t BSP_random(void) {
    // Simple linear congruential generator
    l_rndSeed = l_rndSeed * 1103515245U + 12345U;
    return l_rndSeed;
}

void BSP_randomSeed(uint32_t seed) {
    l_rndSeed = seed;
}

//============================================================================
// HARDWARE ABSTRACTION FUNCTIONS
//============================================================================

void BSP_gpio_init(void) {
    // GPIO initialization is handled in GPIO_Init()
    // This function can be used for application-specific GPIO setup
    
    // {{APPLICATION_GPIO_INIT}}
}

void BSP_uart_init(void) {
    // UART initialization for application use
    // QS UART is initialized separately
    
    // {{APPLICATION_UART_INIT}}
}

void BSP_spi_init(void) {
    // SPI initialization for application use
    
    // {{APPLICATION_SPI_INIT}}
}

void BSP_timer_init(void) {
    // Timer initialization for application use
    
    // {{APPLICATION_TIMER_INIT}}
}

//============================================================================
// ERROR HANDLING
//============================================================================

void BSP_error_handler(ErrorCode_t error, const char* file, int line) {
    // Disable interrupts
    __disable_irq();
    
    // Turn on error LED
    BSP_ledOn(3);
    
    // QS trace error
    QS_BEGIN_ID(QS_ERROR_INFO, 0U)
        QS_U16_((uint16_t)error);
        QS_STR_(file);
        QS_U16_((uint16_t)line);
    QS_END_()
    
    // In debug mode, break into debugger
#ifdef DEBUG
    __BKPT(0);
#endif
    
    // Reset system after delay
    HAL_Delay(1000);
    NVIC_SystemReset();
}

//============================================================================
// LOCAL FUNCTIONS IMPLEMENTATION
//============================================================================

static void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    // Initializes the RCC Oscillators according to the specified parameters
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;   // 8MHz HSE / 8 = 1MHz
    RCC_OscInitStruct.PLL.PLLN = 336; // 1MHz * 336 = 336MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // 336MHz / 2 = 168MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;   // 336MHz / 7 = 48MHz (USB)
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // Initializes the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 168MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;    // 42MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;    // 84MHz
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

static void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    LED_GPIO_CLK_ENABLE();
    BUTTON_GPIO_CLK_ENABLE();
    
    // Configure LED pins
    GPIO_InitStruct.Pin = LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure button pin
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_Rising;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);
    
    // Enable button interrupt
    HAL_NVIC_SetPriority(BUTTON_EXTI_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1U, 0);
    HAL_NVIC_EnableIRQ(BUTTON_EXTI_IRQn);
}

static void UART_Init(void) {
#ifdef Q_SPY
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable clocks
    QS_UART_CLK_ENABLE();
    QS_UART_GPIO_CLK_ENABLE();
    
    // Configure UART GPIO pins
    GPIO_InitStruct.Pin = QS_UART_TX_PIN | QS_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = QS_UART_AF;
    HAL_GPIO_Init(QS_UART_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure UART
    l_uartHandle.Instance = QS_UART;
    l_uartHandle.Init.BaudRate = QS_UART_BAUDRATE;
    l_uartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    l_uartHandle.Init.StopBits = UART_STOPBITS_1;
    l_uartHandle.Init.Parity = UART_PARITY_NONE;
    l_uartHandle.Init.Mode = UART_MODE_TX_RX;
    l_uartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    l_uartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&l_uartHandle) != HAL_OK) {
        Error_Handler();
    }
#endif
}

static void Error_Handler(void) {
    // Disable interrupts
    __disable_irq();
    
    // Turn on all LEDs to indicate error
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN, GPIO_PIN_SET);
    
    // Infinite loop
    while (1) {
        __NOP();
    }
}

//============================================================================
// INTERRUPT SERVICE ROUTINES
//============================================================================

void EXTI0_IRQHandler(void) {
    // QK-aware interrupt entry
    QK_ISR_ENTRY();
    
    // Handle button interrupt
    if (__HAL_GPIO_EXTI_GET_IT(BUTTON_PIN) != 0x00U) {
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_PIN);
        
        // Post button event to system
        static QEvt const buttonEvt = QEVT_INITIALIZER(GPIO_SIG);
        QACTIVE_PUBLISH(&buttonEvt, &l_EXTI0_IRQHandler);
        
        // QS trace
        QS_BEGIN_ID(QS_GPIO_CHANGE, 0U)
            QS_U8_(0xFF);  // Button ID
            QS_U8_(1U);    // Pressed
        QS_END_()
    }
    
    // QK-aware interrupt exit
    QK_ISR_EXIT();
}

void BSP_systick_handler(void) {
    // This function is called from SysTick_Handler
    // which is already QK-aware in main.c
    
    // Additional system tick processing can be added here
    // {{ADDITIONAL_SYSTICK_PROCESSING}}
}

//============================================================================
// QS SOFTWARE TRACING IMPLEMENTATION
//============================================================================

#ifdef Q_SPY

void QS_onStartup(void) {
    // QS startup initialization
    // UART is already initialized in BSP_init()
}

void QS_onCleanup(void) {
    // QS cleanup
}

void QS_onFlush(void) {
    // Flush QS transmit buffer via UART
    uint16_t nBytes = QS_TX_BUFFER_SIZE;
    uint8_t const *data = QS_getBlock(&nBytes);
    
    while (nBytes != 0U) {
        // Transmit data via UART
        HAL_UART_Transmit(&l_uartHandle, (uint8_t *)data, nBytes, 100);
        
        // Get next block
        nBytes = QS_TX_BUFFER_SIZE;
        data = QS_getBlock(&nBytes);
    }
}

QSTimeCtr QS_onGetTime(void) {
    // Return current timestamp for QS
    return (QSTimeCtr)BSP_getTimeUs();
}

void QS_onCommand(uint8_t cmdId, uint32_t param1, 
                  uint32_t param2, uint32_t param3) {
    // Handle QS-RX commands
    switch (cmdId) {
        case 0U: {
            // Command 0: Toggle LED
            BSP_ledToggle((uint8_t)param1);
            break;
        }
        
        case 1U: {
            // Command 1: Set LED state
            if (param2 != 0U) {
                BSP_ledOn((uint8_t)param1);
            } else {
                BSP_ledOff((uint8_t)param1);
            }
            break;
        }
        
        case 2U: {
            // Command 2: System reset
            NVIC_SystemReset();
            break;
        }
        
        case 3U: {
            // Command 3: Set random seed
            BSP_randomSeed(param1);
            break;
        }
        
        // {{CUSTOM_QS_COMMANDS}}
        
        default: {
            break;
        }
    }
}

// Override default QS RX input
void QS_RX_PUT(uint8_t b) {
    // Receive byte from UART and put into QS RX buffer
    QS_rxPut(b);
}

#endif // Q_SPY

//============================================================================
// HAL CALLBACK FUNCTIONS
//============================================================================

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // GPIO interrupt callback
    if (GPIO_Pin == BUTTON_PIN) {
        // Button press handled in IRQ handler
        // Additional processing can be added here
    }
}

//============================================================================
// TEMPLATE EXPANSION MARKERS
//============================================================================

// These markers are used by code generation tools:
// {{ADDITIONAL_BSP_FUNCTIONS}}
// {{PLATFORM_SPECIFIC_HARDWARE}}
// {{PROJECT_SPECIFIC_BSP_CODE}}

/**
 * @brief Template Usage Instructions for AI Agents
 * 
 * This BSP template provides:
 * 
 * 1. Complete STM32F4 hardware initialization
 * 2. LED control functions for status indication
 * 3. System timing and random number generation
 * 4. QS software tracing integration
 * 5. QK-aware interrupt handling
 * 6. Error handling and recovery
 * 
 * To customize for specific projects:
 * 
 * 1. Update GPIO pin definitions for target board
 * 2. Add application-specific hardware initialization
 * 3. Implement custom QS commands for debugging
 * 4. Add periodic processing in BSP_tickHook()
 * 5. Extend error handling for application needs
 * 
 * QK Integration Points:
 * - All IRQ handlers use QK_ISR_ENTRY()/QK_ISR_EXIT()
 * - Event posting from ISRs follows QK patterns
 * - Timing functions support real-time constraints
 * - Hardware abstraction maintains non-blocking semantics
 * 
 * Template Variables to Replace:
 * - {{PERIODIC_BSP_PROCESSING}}: Periodic BSP tasks
 * - {{APPLICATION_*_INIT}}: Application-specific initialization
 * - {{ADDITIONAL_SYSTICK_PROCESSING}}: Extra system tick handling
 * - {{CUSTOM_QS_COMMANDS}}: Project-specific QS commands
 * - {{ADDITIONAL_BSP_FUNCTIONS}}: Extra BSP functions
 * - {{PLATFORM_SPECIFIC_HARDWARE}}: Hardware-specific code
 * - {{PROJECT_SPECIFIC_BSP_CODE}}: Application BSP code
 */