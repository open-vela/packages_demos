#
# Copyright (C) 2022 Xiaomi Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

-include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_VELA_AUDIO), y)
PROGNAME = music_player2
PRIORITY = 100
STACKSIZE = 16384  # Reduce stack size from 40KB to 16KB
MODULE = $(CONFIG_LVX_USE_DEMO_VELA_AUDIO)

# Music player source files - using simulator compatible version
CSRCS = music_player2.c audio_ctl.c wifi.c splash_screen.c playlist_manager.c font_config.c

# Main entry file
MAINSRC = music_player2_main.c

# Simulator environment configuration
CFLAGS += -DUSING_SIMULATOR_AUDIO=1

# Basic library linking
LDLIBS += -lm -lpthread

endif

include $(TOPDIR)/../apps/Application.mk