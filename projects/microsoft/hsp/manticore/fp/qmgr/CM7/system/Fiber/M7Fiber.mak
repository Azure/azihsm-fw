# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include path
#------------------------------------------------------------------------------
INCLUDE_DIR += -IFP3Core/System/Fiber
INCLUDE_DIR += -IFP3Core/ldscripts
#INCLUDE_DIR += -IFP3Core/FCP/
#------------------------------------------------------------------------------
# Object file list
#------------------------------------------------------------------------------
ifneq ($(_CPU_),_Shared)
    M7FIBER_OBJS += $(OBJ_DIR)/FP3Core/System/Fiber/M7FiberScheduler.o
    M7FIBER_OBJS += $(OBJ_DIR)/FP3Core/System/Fiber/M7FiberSchedulerContext.o
    M7FIBER_OBJS += $(OBJ_DIR)/FP3Core/System/Fiber/M7Fiber.o
endif

#------------------------------------------------------------------------------
# Include component make file
#------------------------------------------------------------------------------
