# ##############################################################################
# packages/demos/bailian/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_AI_BAILIAN)

  set(BAILIAN_SDK_DIR ${NUTTX_DIR}/../external/bailian_sdk)

  # HAL layer sources - these implement the SDK's required interfaces
  set(HAL_SRCS
      hal/hal_mem.c
      hal/hal_time.c
      hal/hal_random.c
      hal/hal_storage.c
      hal/hal_mutex.c)

  # Application sources
  set(APP_SRCS
      bailian_main.c
      net/http_client.c
      net/websocket.c
      audio/recorder.c
      audio/player.c)

  # Create HAL library first - this provides symbols needed by SDK
  nuttx_add_library(bailian_hal STATIC)
  target_sources(bailian_hal PRIVATE ${HAL_SRCS})
  target_include_directories(bailian_hal PRIVATE
    ${BAILIAN_SDK_DIR}/include
    ${BAILIAN_SDK_DIR}/include/c_utils)

  # Create the main application
  nuttx_add_application(
    MODULE
    ${CONFIG_AI_BAILIAN}
    NAME
    ${CONFIG_AI_BAILIAN_PROGNAME}
    SRCS
    ${APP_SRCS}
    STACKSIZE
    ${CONFIG_AI_BAILIAN_STACKSIZE}
    PRIORITY
    ${CONFIG_AI_BAILIAN_PRIORITY}
    INCLUDE_DIRECTORIES
    ${BAILIAN_SDK_DIR}/include
    ${BAILIAN_SDK_DIR}/include/c_utils
    ${BAILIAN_SDK_DIR}/third_party/cJSON
    ${BAILIAN_SDK_DIR}/third_party/tinycrypt/include)

  # Link SDK libraries with HAL library using whole-archive to ensure all HAL symbols are included
  target_link_libraries(apps_${CONFIG_AI_BAILIAN_PROGNAME}
    PRIVATE
    -Wl,--whole-archive
    bailian_hal
    -Wl,--no-whole-archive
    -Wl,--start-group
    ${BAILIAN_SDK_DIR}/libs/libqwen_sdk.a
    # libc_license.a removed - using pay-as-you-go mode
    ${BAILIAN_SDK_DIR}/libs/libc_mmi_cmd.a
    ${BAILIAN_SDK_DIR}/libs/libqwen_test.a
    ${BAILIAN_SDK_DIR}/third_party/cJSON/libcjson.a
    ${BAILIAN_SDK_DIR}/third_party/tinycrypt/libtinycrypt.a
    -Wl,--end-group)

endif()
