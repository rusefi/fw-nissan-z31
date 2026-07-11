include $(BOARD_DIR)/firmware/firmware.mk

ifneq ($(PROJECT_CPU),simulator)
BOARDCPPSRC += $(BOARD_DIR)/ext/rusefi/firmware/config/boards/hellen/uaefi121/mega-uaefi.cpp
endif

BOARDINC += $(BOARD_DIR)/generated/controllers/generated \
    $(BOARD_DIR)/ext/rusefi/firmware/config/boards/hellen/uaefi121/

# defines SHORT_BOARD_NAME
include $(BOARD_DIR)/meta-info.env

ifneq ($(PROJECT_CPU),simulator)
include $(BOARD_DIR)/ext/rusefi/firmware/config/boards/hellen/uaefi121/mega-uaefi.mk
endif

# this would save some flash while being unable to update WBO controller firmware
DDEFS += -DEFI_WIDEBAND_FIRMWARE_UPDATE=FALSE

# assign critical LED to a non-existent pin if you do not have it on your board
# good old PD14 is still the default value
# DDEFS += -DLED_CRITICAL_ERROR_BRAIN_PIN=Gpio::I15

# EGT chip
#un-comment to enable
#DDEFS += -DEFI_MAX_31855=TRUE

#see main repo for details on this any many other optional subsystems. We have too many, one has to choose what fits into his choice of stm32
#DDEFS += -DEFI_ONBOARD_MEMS=TRUE
