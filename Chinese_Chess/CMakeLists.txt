# ##############################################################################
# packages/demos/ChineseChess/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
# ##############################################################################

if(CONFIG_LVX_USE_DEMO_CHINESE_CHESS)

  # Application settings
  set(PROGNAME chess)
  set(PRIORITY 100)
  set(STACKSIZE 32768)
  set(MODULE ${CONFIG_LVX_USE_DEMO_CHINESE_CHESS})

  # Main source file
  set(MAINSRC Chinese_chess_main.c)

  # C source files
  set(CSRCS
      Chinese_chess.c
  )

  # Add application to NuttX
  nuttx_add_application(
    NAME ${PROGNAME}
    PRIORITY ${PRIORITY}
    STACKSIZE ${STACKSIZE}
    MODULE ${MODULE}
    SRCS ${MAINSRC} ${CSRCS}
  )

endif()
