# ##############################################################################
# packages/demos/mimo/CMakeLists.txt
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
# ##############################################################################

if(CONFIG_DEMOS_MIMO)
  set(PROGNAME mimo)
  set(PRIORITY ${CONFIG_MIMO_PRIORITY})
  set(STACKSIZE ${CONFIG_MIMO_STACKSIZE})
  set(MODULE ${CONFIG_DEMOS_MIMO})
  set(MAINSRC mimo_main.c)

  set(CSRCS
    mimo_provider.c
    mimo_agent.c
  )

  nuttx_add_application(
    NAME ${PROGNAME}
    PRIORITY ${PRIORITY}
    STACKSIZE ${STACKSIZE}
    MODULE ${MODULE}
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS ${MAINSRC} ${CSRCS}
  )
endif()
