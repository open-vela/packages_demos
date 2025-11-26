# ##############################################################################
# packages/demos/ai_chat/CMakeLists.txt
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

if(CONFIG_AI_CONVERSATION_DEMO)

  # ############################################################################
  # Include Directory
  # ############################################################################

  set(INCDIR ${CMAKE_CURRENT_LIST_DIR}/utils
             ${CMAKE_CURRENT_LIST_DIR}/include
             ${CMAKE_CURRENT_LIST_DIR}/src/conversation/plugin)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/libwebsockets)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/frameworks/system/utils/uv/include)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/libwebsockets/libwebsockets/include)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/external/json-c)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/external/json-c/json-c)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/apps/include)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/frameworks/system/utils/include)
  list(APPEND INCDIR ${NUTTX_APPS_DIR}/apps/include/netutils)
  list(APPEND INCDIR ${CMAKE_CURRENT_LIST_DIR}/src/mcp)

  # ############################################################################
  # Sources
  # ############################################################################

  file(GLOB CSRCS 
      "${CMAKE_CURRENT_SOURCE_DIR}/src/conversation/*.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/conversation/plugin/*.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/conversation/volc/*.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.c"
  )
  
  message("cmake src list:${CSRCS}${CXXSRCS}")

  nuttx_add_application(
    NAME
    ai_conversation
    SRCS
    src/ai_conversation_main.c
    STACKSIZE
    ${CONFIG_AI_CONVERSATION_GUI_STACKSIZE}
    PRIORITY
    ${CONFIG_AI_CONVERSATION_GUI_PRIORITY}
    INCLUDE_DIRECTORIES
    ${INCDIR})

  # ############################################################################
  # Library Configuration
  # ############################################################################

  nuttx_add_library(libai STATIC)
  target_sources(libai PRIVATE ${CSRCS})
  target_include_directories(libai PRIVATE ${INCDIR})

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include)

endif()
