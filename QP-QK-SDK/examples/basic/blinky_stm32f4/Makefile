# Makefile for Blinky STM32F4 Example
# QP-QK SDK Build System

#============================================================================
# Project Configuration
#============================================================================

PROJECT_NAME = blinky
TARGET_PLATFORM = stm32f4
TARGET_MCU = stm32f411re

# Version information
PROJECT_VERSION_MAJOR = 1
PROJECT_VERSION_MINOR = 0
PROJECT_VERSION_PATCH = 0

#============================================================================
# Toolchain Configuration
#============================================================================

# ARM GCC Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
GDB = $(PREFIX)gdb

#============================================================================
# Directory Structure
#============================================================================

# Project directories
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# QP Framework paths (adjust as needed)
QP_DIR = ../../../../qpc
QP_INC_DIR = $(QP_DIR)/include
QP_SRC_DIR = $(QP_DIR)/src
QP_PORT_DIR = $(QP_DIR)/ports/arm-cm/qk/gnu

# HAL paths (STM32CubeF4)
HAL_DIR = ../../../../STM32Cube_FW_F4_V1.27.0
HAL_INC_DIR = $(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc
HAL_SRC_DIR = $(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Src
CMSIS_INC_DIR = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CMSIS_CORE_INC_DIR = $(HAL_DIR)/Drivers/CMSIS/Include

#============================================================================
# Source Files
#============================================================================

# Project source files
PROJECT_SRCS = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/blinky.c \
	$(wildcard $(SRC_DIR)/*.c)

# BSP source files
BSP_SRCS = \
	../../../templates/platforms/stm32f4/bsp.c

# QP Framework source files
QP_SRCS = \
	$(QP_SRC_DIR)/qf/qf_act.c \
	$(QP_SRC_DIR)/qf/qf_actq.c \
	$(QP_SRC_DIR)/qf/qf_defer.c \
	$(QP_SRC_DIR)/qf/qf_dyn.c \
	$(QP_SRC_DIR)/qf/qf_mem.c \
	$(QP_SRC_DIR)/qf/qf_ps.c \
	$(QP_SRC_DIR)/qf/qf_qact.c \
	$(QP_SRC_DIR)/qf/qf_qeq.c \
	$(QP_SRC_DIR)/qf/qf_qmact.c \
	$(QP_SRC_DIR)/qf/qf_time.c \
	$(QP_SRC_DIR)/qk/qk.c \
	$(QP_SRC_DIR)/qs/qs.c \
	$(QP_SRC_DIR)/qs/qs_64bit.c \
	$(QP_SRC_DIR)/qs/qs_rx.c \
	$(QP_SRC_DIR)/qs/qs_fp.c \
	$(QP_PORT_DIR)/qf_port.c

# HAL source files (minimal set)
HAL_SRCS = \
	$(HAL_SRC_DIR)/stm32f4xx_hal.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_cortex.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_gpio.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_rcc.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_uart.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_pwr.c \
	$(HAL_SRC_DIR)/stm32f4xx_hal_pwr_ex.c

# Startup file
STARTUP_SRC = $(CMSIS_INC_DIR)/../Source/Templates/gcc/startup_stm32f411xe.s

# All source files
ALL_SRCS = $(PROJECT_SRCS) $(BSP_SRCS) $(QP_SRCS) $(HAL_SRCS) $(STARTUP_SRC)

#============================================================================
# Include Directories
#============================================================================

INCLUDES = \
	-I$(INC_DIR) \
	-I$(QP_INC_DIR) \
	-I$(QP_PORT_DIR) \
	-I$(HAL_INC_DIR) \
	-I$(HAL_INC_DIR)/Legacy \
	-I$(CMSIS_INC_DIR) \
	-I$(CMSIS_CORE_INC_DIR)

#============================================================================
# Compiler Flags
#============================================================================

# MCU specific flags
MCU_FLAGS = \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard

# Optimization flags
OPTIMIZATION = -Os
DEBUG_FLAGS = -g -DDEBUG

# Warning flags
WARNING_FLAGS = \
	-Wall \
	-Wextra \
	-Wstrict-prototypes \
	-Wno-unused-parameter

# Language flags
LANG_FLAGS = \
	-std=c99 \
	-ffunction-sections \
	-fdata-sections

# Preprocessor defines
DEFINES = \
	-DUSE_HAL_DRIVER \
	-DSTM32F411xE \
	-DQP_API_VERSION=720 \
	-DQ_SPY=1 \
	-DAPP_VERSION_MAJOR=$(PROJECT_VERSION_MAJOR) \
	-DAPP_VERSION_MINOR=$(PROJECT_VERSION_MINOR) \
	-DAPP_VERSION_PATCH=$(PROJECT_VERSION_PATCH)

# Combined CFLAGS
CFLAGS = $(MCU_FLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS) $(WARNING_FLAGS) \
         $(LANG_FLAGS) $(DEFINES) $(INCLUDES)

# Assembler flags
ASFLAGS = $(MCU_FLAGS) $(DEBUG_FLAGS)

#============================================================================
# Linker Flags
#============================================================================

# Linker script
LINKER_SCRIPT = STM32F411RETx_FLASH.ld

# Linker flags
LDFLAGS = \
	$(MCU_FLAGS) \
	-specs=nano.specs \
	-specs=nosys.specs \
	-T$(LINKER_SCRIPT) \
	-Wl,-Map=$(BUILD_DIR)/$(PROJECT_NAME).map,--cref \
	-Wl,--gc-sections \
	-Wl,--print-memory-usage

# Libraries
LIBS = -lc -lm -lnosys

#============================================================================
# Build Targets
#============================================================================

# Object files
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(ALL_SRCS:.c=.o)))
OBJS := $(OBJS:.s=.o)

# Default target
all: $(BUILD_DIR)/$(PROJECT_NAME).elf $(BUILD_DIR)/$(PROJECT_NAME).bin \
     $(BUILD_DIR)/$(PROJECT_NAME).hex size

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Compile C source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile BSP source files
$(OBJ_DIR)/%.o: ../../../templates/platforms/stm32f4/%.c | $(OBJ_DIR)
	@echo "Compiling BSP $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile QP source files
$(OBJ_DIR)/%.o: $(QP_SRC_DIR)/qf/%.c | $(OBJ_DIR)
	@echo "Compiling QP/QF $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(QP_SRC_DIR)/qk/%.c | $(OBJ_DIR)
	@echo "Compiling QP/QK $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(QP_SRC_DIR)/qs/%.c | $(OBJ_DIR)
	@echo "Compiling QP/QS $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(QP_PORT_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling QP port $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile HAL source files
$(OBJ_DIR)/%.o: $(HAL_SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling HAL $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile startup file
$(OBJ_DIR)/%.o: $(CMSIS_INC_DIR)/../Source/Templates/gcc/%.s | $(OBJ_DIR)
	@echo "Assembling $<"
	$(CC) $(ASFLAGS) -c $< -o $@

# Link the executable
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(OBJS) | $(BUILD_DIR)
	@echo "Linking $@"
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

# Create binary file
$(BUILD_DIR)/$(PROJECT_NAME).bin: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Creating binary $@"
	$(OBJCOPY) -O binary $< $@

# Create hex file
$(BUILD_DIR)/$(PROJECT_NAME).hex: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Creating hex $@"
	$(OBJCOPY) -O ihex $< $@

# Create disassembly
$(BUILD_DIR)/$(PROJECT_NAME).asm: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Creating disassembly $@"
	$(OBJDUMP) -d $< > $@

# Display size information
size: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Memory usage:"
	$(SIZE) -A $<

#============================================================================
# Utility Targets
#============================================================================

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Flash using ST-Link
flash: $(BUILD_DIR)/$(PROJECT_NAME).bin
	st-flash write $< 0x08000000

# Erase flash
erase:
	st-flash erase

# Reset target
reset:
	st-flash reset

# Start GDB session
debug: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(GDB) $<

# Show target info
info:
	st-info --probe

# QSpy session (requires QSpy to be in PATH)
qspy:
	qspy -c /dev/ttyACM0 -b 115200

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the project (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  flash   - Flash firmware to target"
	@echo "  erase   - Erase target flash"
	@echo "  reset   - Reset target"
	@echo "  debug   - Start GDB session"
	@echo "  info    - Show target information"
	@echo "  qspy    - Start QSpy trace session"
	@echo "  size    - Show memory usage"
	@echo "  help    - Show this help"

# Declare phony targets
.PHONY: all clean flash erase reset debug info qspy size help

#============================================================================
# Dependencies
#============================================================================

# Include dependency files
-include $(OBJS:.o=.d)

# Generate dependency files
$(OBJ_DIR)/%.d: %.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@