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

ifeq ($(CONFIG_LVX_USE_APPS_WATCH), y)
PROGNAME = watch
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_APPS_WATCH)

# CSRCS = $(wildcard src/*.c res/images/*.c)
CSRCS = $(wildcard main_page/src/*.c) $(wildcard main_page/res/images/*.c)
CSRCS += $(wildcard message/src/*.c) $(wildcard message/res/*.c)
CSRCS += $(wildcard launcher/src/*.c) $(wildcard launcher/res/*.c)
CSRCS += $(wildcard timer/src/*.c) $(wildcard timer/res/*.c)
# CSRCS += $(wildcard clock/src/*.c) $(wildcard clock/res/*.c)
MAINSRC = watch_main.c
$(info 当前路径是：$(shell pwd))
$(warning $(CSRCS))

CFLAGS +=-Imain_page/include
VPATH  += main_page/src main_page/res/images

CFLAGS +=-Imessage/include
VPATH  += message/src message/res


CFLAGS +=-Ilauncher/include
VPATH  += launcher/src launcher/res

CFLAGS +=-Itimer/include
VPATH  += timer/src timer/res

# CFLAGS +=-Iclock/include
# VPATH  += clock/src clock/res

endif

include $(APPDIR)/Application.mk
