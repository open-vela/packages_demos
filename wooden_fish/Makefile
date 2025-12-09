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

include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_DEMO_WOODEN_FISH), y)
PROGNAME = wooden_fish
PRIORITY = 100
STACKSIZE = 131072
MODULE = $(CONFIG_LVX_USE_DEMO_WOODEN_FISH)

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/utils/include
CSRCS = wooden_fish.c
MAINSRC = wooden_fish_main.c 
endif

include $(APPDIR)/Application.mk

