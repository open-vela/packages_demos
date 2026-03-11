############################################################################
# packages/demos/mimo/Makefile
#
# Copyright (C) 2024 Xiaomi Corporation
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
############################################################################

include $(APPDIR)/Make.defs

ifeq ($(CONFIG_DEMOS_MIMO), y)
PROGNAME  = mimo
PRIORITY  = $(CONFIG_MIMO_PRIORITY)
STACKSIZE = $(CONFIG_MIMO_STACKSIZE)
MODULE    = $(CONFIG_DEMOS_MIMO)

CSRCS  = mimo_provider.c
CSRCS += mimo_agent.c

MAINSRC = mimo_main.c

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/mimo

endif

include $(APPDIR)/Application.mk
