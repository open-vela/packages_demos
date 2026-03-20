############################################################################
# packages/demos/bailian/Makefile
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

PROGNAME  = $(CONFIG_AI_BAILIAN_PROGNAME)
PRIORITY  = $(CONFIG_AI_BAILIAN_PRIORITY)
STACKSIZE = $(CONFIG_AI_BAILIAN_STACKSIZE)
HEAPSIZE  = $(CONFIG_AI_BAILIAN_HEAPSIZE)
MODULE    = $(CONFIG_AI_BAILIAN)

MAINSRC = bailian_main.c

CSRCS = \
	hal/hal_mem.c \
	hal/hal_time.c \
	hal/hal_random.c \
	hal/hal_storage.c \
	hal/hal_mutex.c \
	net/http_client.c \
	net/websocket.c \
	audio/recorder.c \
	audio/player.c

BAILIAN_DIR = $(APPDIR)/../external/bailian_sdk
SDK_INCDIR = $(BAILIAN_DIR)/include
SDK_LIBDIR = $(BAILIAN_DIR)/libs

CFLAGS += ${INCDIR_PREFIX}"$(SDK_INCDIR)"
CFLAGS += ${INCDIR_PREFIX}"$(SDK_INCDIR)/c_utils"
CFLAGS += ${INCDIR_PREFIX}"$(APPDIR)/include/netutils"

# Copy SDK libraries to staging directory for linking
context::
	$(Q) cp -f $(SDK_LIBDIR)/libqwen_sdk.a $(APPDIR)/staging/
	$(Q) # libc_license.a removed - using pay-as-you-go mode
	$(Q) cp -f $(SDK_LIBDIR)/libc_mmi_cmd.a $(APPDIR)/staging/
	$(Q) cp -f $(SDK_LIBDIR)/libc_mmi_cmd_application.a $(APPDIR)/staging/
	$(Q) cp -f $(SDK_LIBDIR)/libc_mmi_cmd_voice_translate.a $(APPDIR)/staging/
	$(Q) cp -f $(SDK_LIBDIR)/libc_mmi_cmd_volume.a $(APPDIR)/staging/
	$(Q) cp -f $(SDK_LIBDIR)/libc_visual.a $(APPDIR)/staging/
	$(Q) cp -f $(BAILIAN_DIR)/third_party/tinycrypt/libtinycrypt.a $(APPDIR)/staging/
	$(Q) cp -f $(SDK_LIBDIR)/libqwen_test.a $(APPDIR)/staging/

# Link SDK libraries
LDLIBS += -lqwen_test

include $(APPDIR)/Application.mk
