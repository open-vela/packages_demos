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

ifeq ($(CONFIG_LVX_USE_DEMO_RELATIVES_CALCULATOR), y)
PROGNAME = rel_cal
PRIORITY = 100
STACKSIZE = 131072
MODULE = $(CONFIG_LVX_USE_DEMO_RELATIVES_CALCULATOR)

MAINSRC = relation_cal_main.c
CSRCS += relation_cal.c
ifeq ($(CONFIG_ESP32S3_BOX_LCD), y)
CSRCS += ./fonts/lv_font_siyuan_10.c
else
CSRCS += ./fonts/lv_font_siyuan_16.c
endif

include $(APPDIR)/Application.mk

endif