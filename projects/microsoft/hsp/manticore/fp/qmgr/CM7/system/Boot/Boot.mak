# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include path
#------------------------------------------------------------------------------
INCLUDE_DIR += -IFP3Core/System/Boot
#------------------------------------------------------------------------------
# Object file list
#------------------------------------------------------------------------------

M7_BOOT_OBJS     += $(OBJ_DIR)/FP3Core/System/Boot/FpsCpu$(_CPU_)Boot.o
M7_BOOT_OBJS     += $(OBJ_DIR)/FP3Core/System/Boot/FpsCpu$(_CPU_)Main.o

#------------------------------------------------------------------------------
# Include component make file
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
ifeq ($(_CPU_),0)
    $(OBJ_DIR)/FP3Core/System/Boot/FpsCpu$(_CPU_)Main.o: INCLUDE_DIR += $(FPSCPU0_INCLUDE_DIR)
else
    $(OBJ_DIR)/FP3Core/System/Boot/FpsCpu$(_CPU_)Main.o: INCLUDE_DIR += $(FPSCPU1_INCLUDE_DIR)
endif
#$(OBJ_DIR)/System/Boot/ObjectInitializerShared.o : C_CORE += -mthumb
