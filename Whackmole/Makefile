# Makefile


include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_WHACKMOLE), y)
PROGNAME = Whackmole
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_DEMO_WHACKMOLE)
CSRCS = Whackmole.c
MAINSRC = Whackmole_main.c
endif

include $(APPDIR)/Application.mk
