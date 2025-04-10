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

ifeq ($(CONFIG_LVX_USE_DEMO_CALCULATOR), y)
PROGNAME = calculator
PRIORITY = 100
STACKSIZE = 32768
MODULE = $(CONFIG_LVX_USE_DEMO_CALCULATOR)

CSRCS = calculator.c 
MAINSRC = calculator_main.c
endif

include $(APPDIR)/Application.mk
