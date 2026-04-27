# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#
# Common Stuff
#
MAKECONF_DIR := $(shell cd ../../ && pwd)
NAME:=fps_cpu0
CORTEX_CORE:=m7
include ../../makefile.conf


INC_OPTS +=-I$(MAKECONF_DIR)/CM7/src -I$(MAKECONF_DIR)/hal/Common -I$(MAKECONF_DIR)/hal/CDMA -I$(MAKECONF_DIR)/hal/MQ -I$(MAKECONF_DIR)/hal/UCD -I$(MAKECONF_DIR)/hal/FPS -I$(MAKECONF_DIR)/hal/Cortexm7 -I$(MAKECONF_DIR)/hal/TCON -I$(CURDIR)/src -I$(MAKECONF_DIR)/CM7/system/Fiber -I$(MAKECONF_DIR)/CM7/system/Debug -I$(MAKECONF_DIR)/CM7/system/Heartbeat/ -I$(MAKECONF_DIR)/CM7/system -I$(MAKECONF_DIR)/CM7/system/Boot/ -I$(MAKECONF_DIR)/CM7/ -I$(MAKECONF_DIR)/CM7/Common -I$(MAKECONF_DIR)/CM7/Inc -I$(MAKECONF_DIR)/CM7/ldscripts/ -I$(CURDIR) -I$(MAKECONF_DIR)/CM7/system

#
# Make default target
#  Note: need $$ ALL_OBJ to allow target name expanded on first pass.
all: info output yaml kconfig
all: rebuild_debug_obj rebuild_fiber_obj rebuild_heartbeat_obj $$(ALL_OBJ) $(NAME).axf

# Remove previous (if have) LoggingDebug.o to trigger rebuild
rebuild_debug_obj:
	rm -f $(OSYSDIR)/Debug/LoggingDebug.o
rebuild_fiber_obj:
	rm -f $(OSYSDIR)/Fiber/M7FiberScheduler.o
rebuild_heartbeat_obj:
	rm -f $(OSYSDIR)/Heartbeat/Heartbeat.o

ALL_OBJ=$(STARTUP_OBJ) $(ARCH_COM_OBJ) $(COM_OBJ) $(HAL_FPS_OBJ) $(HAL_CDMA_OBJ) $(SRC_OBJ)


#
# Startup files
#
STARTUP= vectors.S \
	 boot_loader.S \
	 init.S

STARTUP_OBJ= $(patsubst %.S, $(ODIR)/%.o, $(STARTUP))
STARTUP_DIR= $(CURDIR)/../startup
STARTUP_OPT=

$(eval $(call S_STATIC_RULE,$(STARTUP_OBJ),$(STARTUP_DIR),$(STARTUP_OPT)))


#
# Arch Common C files
#
ARCH_COM= system_cm7ikmcu.c irq.c vicommon.c crashdump.c ../system/Debug/logging/logging_producer.c APILogging.c API_GCMTagCorrect.c

ARCH_COM_OBJ = $(patsubst %.c, $(ODIR)/%.o, $(ARCH_COM))
ARCH_COM_DIR = $(CURDIR)/../src
ARCH_COM_OPT = -DCPU0

$(eval $(call C_STATIC_RULE,$(ARCH_COM_OBJ),$(ARCH_COM_DIR),$(ARCH_COM_OPT)))

#
# HAL C files
#
HAL_CDMA = APICdma.c HalCdma.c
HAL_CDMA_OBJ = $(patsubst %.c, $(ODIR)/%.o, $(HAL_CDMA))
HAL_CDMA_DIR = $(CURDIR)/../../hal/CDMA
HAL_OPT =
$(eval $(call C_STATIC_RULE,$(HAL_CDMA_OBJ),$(HAL_CDMA_DIR),$(HAL_OPT)))

HAL_FPS= HalFps.c
HAL_FPS_OBJ = $(patsubst %.c, $(ODIR)/%.o, $(HAL_FPS))
HAL_FPS_DIR = $(CURDIR)/../../hal/FPS
HAL_OPT =
$(eval $(call C_STATIC_RULE,$(HAL_FPS_OBJ),$(HAL_FPS_DIR),$(HAL_OPT)))

#
# Source C files
#

SRC_DIR ?= $(CURDIR)
$(info  $(SRC_DIR))
SRC ?= $(shell find ../system ../Common/ ../ldscripts/ . -name '*.cpp' -a ! -name '*1*.cpp' -a ! -name '*2*.cpp')
$(info  $(SRC))
SRC_OBJ = $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))
$(info  $(SRC_OBJ))
SRC_OPT ?= -DCPU0

$(eval $(call CPP_STATIC_RULE,$(SRC_OBJ),$(SRC_DIR),$(SRC_OPT)))


#
# AXF (ARM Executable Format)
#
$(NAME).axf:
	@echo ; \
        echo "$(THIS_MAKEFILE): ****** Building $@...";
	cd $(ODIR); \
	$(CC) -x c -E -P $(CFLAGS) $(CURDIR)/../../ldscripts/$(NAME).lds -o $(NAME).ld; \
	$(LD) $(ALL_OBJ) $(LFLAGS) -T $(NAME).ld -o $@; \
	$(OBJCOPY) $(COPYFLAGS_BIN) -j .text -j normal_code  $@ $(NAME).text.bin; \
	$(OBJCOPY) $(COPYFLAGS_BIN) -j .data -j .bss $@ $(NAME).data.bin; \
	$(OBJDUMP) $(DUMPFLAGS) $@ > $(NAME).list; \
	$(SIZE) $@ > $(NAME).size
