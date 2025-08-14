include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_BANDX), y)
PROGNAME += bandx
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_DEMO_BANDX)

MAINSRC += bandx_demo_main.c
CSRCS += $(shell find -L ./ -name "*.c" | grep -v bandx_demo_main.c)

endif

include $(APPDIR)/Application.mk
