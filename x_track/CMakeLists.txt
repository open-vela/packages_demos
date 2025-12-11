# ##############################################################################
# packages/demos/CMakeLists.txt
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

if(CONFIG_LVX_USE_DEMO_X_TRACK)
  # Source Directories
  set(X_TRACK_DIR ${CMAKE_CURRENT_SOURCE_DIR})

  # Flags
  set(INCDIR)

  set(CXXEXT .cpp)
  set(PROGNAME x_track)
  set(PRIORITY 100)
  set(STACKSIZE 32768)
  set(MODULE ${CONFIG_LVX_USE_DEMO_X_TRACK})
  # Absolute page size mode
  add_compile_definitions(PAGE_HOR_RES=240 PAGE_VER_RES=320)
  # Map resource path configuration
  add_compile_definitions(CONFIG_RESOURCE_DIR_PATH=CONFIG_X_TRACK_RESOURCE_DIR_PATH)
  # Log level configuration
  add_compile_definitions(DATA_BROKER_LOG_LEVEL=1 PAGE_LOG_LEVEL=0)
  # Add include dir
  list(APPEND INCDIR ${CMAKE_BINARY_DIR}/apps/include/netutils)
  list(APPEND INCDIR ${X_TRACK_DIR}/../../../frameworks/graphics/uikit/include)
  list(APPEND INCDIR ${X_TRACK_DIR}/src)
  list(APPEND INCDIR ${X_TRACK_DIR}/src/App)
  list(APPEND INCDIR ${X_TRACK_DIR}/src/Vendor/Simulator)

  set(MAINSRC x_track_main.cpp)
  file(GLOB_RECURSE CSRCS "*.c")
  file(GLOB_RECURSE CSRCS_REMOVE "*/itoa.c")
  list(REMOVE_ITEM CSRCS ${CSRCS_REMOVE})
  file(GLOB_RECURSE CXXSRCS_ALL "*.cpp")
  list(FILTER CXXSRCS_ALL EXCLUDE REGEX "x_track_main.cpp")
  set(CXXSRCS ${CXXSRCS_ALL})

  nuttx_add_application(
    NAME ${PROGNAME}
    PRIORITY ${PRIORITY}
    INCLUDE_DIRECTORIES ${INCDIR}
    STACKSIZE ${STACKSIZE}
    MODULE ${MODULE}
    SRCS ${MAINSRC} ${CSRCS} ${CXXSRCS}
  )
endif()
