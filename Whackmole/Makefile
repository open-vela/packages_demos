# Makefile


include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_WHACKMOLE), y)
PROGNAME = Whackmole
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_DEMO_WHACKMOLE)
CSRCS = Whackmole.c pic/MiSans_Normal.c pic/grassland.c pic/hammer_image.c pic/mole_image.c

MAINSRC = Whackmole_main.c
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/Whackmole/pic

endif

include $(APPDIR)/Application.mk
