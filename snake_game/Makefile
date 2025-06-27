include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_SNAKE_GAME), y)
PROGNAME = snake_game
PRIORITY = 100
STACKSIZE = 131072
MODULE = $(CONFIG_LVX_USE_DEMO_SNAKE_GAME)

CSRCS = snake_game.c
MAINSRC = snake_game_main.c
endif

include $(APPDIR)/Application.mk

