include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_X_TRACK), y)

CXXEXT   := .cpp

PROGNAME += x_track
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_DEMO_X_TRACK)

# Absolute page size mode
CXXFLAGS += -DPAGE_HOR_RES=240 -DPAGE_VER_RES=320

# Map resource path configuration
CXXFLAGS += -DCONFIG_RESOURCE_DIR_PATH=CONFIG_X_TRACK_RESOURCE_DIR_PATH

# Log level configuration
CXXFLAGS += -DDATA_BROKER_LOG_LEVEL=1 -DPAGE_LOG_LEVEL=0

# Include paths
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/include/netutils
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/graphics/uikit/include
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/x_track/src
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/x_track/src/App
CXXFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/x_track/src/Vendor/Simulator

MAINSRC  += x_track_main.cpp
CSRCS    += $(shell find -L ./ -name "*.c")
CXXSRCS  += $(shell find -L ./ -name "*.cpp" | grep -v x_track_main.cpp)
endif # CONFIG_LVX_USE_DEMO_X_TRACK

include $(APPDIR)/Application.mk
