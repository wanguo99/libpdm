# Target module name
TARGET ?= test

# PATH CONFIG
ROOT_PATH := $(shell pwd)
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := _build
KDIR ?= /lib/modules/$(shell uname -r)/build

ifeq ($(KERNELRELEASE),) # Makefile

default:
	$(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(ROOT_PATH) modules

clean:
	$(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(ROOT_PATH) clean
	@rm -rf $(BUILD_DIR)

else # KBuild

obj-m := $(TARGET).o

$(TARGET)-objs := $(SRC_DIR)/proc.o

# Use relative path for include directory
ccflags-y := -I$(ROOT_PATH)/$(INCLUDE_DIR)

endif
