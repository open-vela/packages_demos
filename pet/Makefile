include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_PET),y)
PROGNAME  = pet
PRIORITY  = 100
STACKSIZE = 32768
MODULE    = $(CONFIG_LVX_USE_DEMO_PET)

CSRCS = pet.c
MAINSRC = pet_main.c
endif

include $(APPDIR)/Application.mk
