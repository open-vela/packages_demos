############################################################################
# packages/demos/ai_chat/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

MODULE  = y
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/ai_chat/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/system/utils/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/ai_chat/include/mcp/core
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/ai_chat/utils
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/packages/demos/ai_chat/src/conversation/plugin
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/netutils/libwebsockets
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/json-c
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/json-c/json-c
CFLAGS += ${INCDIR_PREFIX}${APPDIR}/apps/include
CFLAGS += ${INCDIR_PREFIX}${APPDIR}/apps/include/netutils

ifneq ($(CONFIG_AI_CONVERSATION_DEMO),)
  CSRCS += $(wildcard src/conversation/*.c)
  CSRCS += $(wildcard src/conversation/plugin/*.c)
  CSRCS += $(wildcard src/conversation/volc/*.c)
  CSRCS += $(wildcard utils/*.c)
  MAINSRC   += src/ai_conversation_main.c
  PROGNAME  += ai_conversation
  PRIORITY  += $(CONFIG_AI_CONVERSATION_GUI_PRIORITY)
  STACKSIZE += $(CONFIG_AI_CONVERSATION_GUI_STACKSIZE)
endif

CSRCS := $(wildcard $(CSRCS))
MAINSRC := $(wildcard $(MAINSRC))
NOEXPORTSRCS = $(ASRCS)$(CSRCS)$(CXXSRCS)$(MAINSRC)

ifneq ($(NOEXPORTSRCS),)
BIN := $(APPDIR)/staging/libai.a
endif

EXPORT_FILES := include

include $(APPDIR)/Application.mk
