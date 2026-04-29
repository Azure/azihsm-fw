# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include path
#------------------------------------------------------------------------------
INCLUDE_DIR += -I$(HAL_DIR)/CDMA/

#------------------------------------------------------------------------------
# Object file list
#------------------------------------------------------------------------------
HAL_CDMA_OBJS  =

HAL_CDMA_OBJS  += $(HAL_OBJ_DIR)/$(HAL_DIR)/CDMA/APICdma.o

HAL_CDMA_OBJS  += $(HAL_OBJ_DIR)/$(HAL_DIR)/CDMA/HalCdma.o

HAL_CDMA_OBJS  += $(HAL_OBJ_DIR)/$(HAL_DIR)/CDMA/APICdmaErrorHandle.o
