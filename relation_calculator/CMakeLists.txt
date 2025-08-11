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

if(CONFIG_LVX_USE_DEMO_RELATIVES_CALCULATOR)
  set(PROGNAME rel_cal)
  set(PRIORITY 100)
  set(STACKSIZE 131072)
  set(MODULE ${CONFIG_LVX_USE_DEMO_RELATIVES_CALCULATOR})
  set(MAINSRC relation_cal_main.c)
  set(CSRCS relation_cal.c)
  if(CONFIG_ESP32S3_BOX_LCD)
    list(APPEND CSRCS ./fonts/lv_font_siyuan_10.c)
  else()
    list(APPEND CSRCS ./fonts/lv_font_siyuan_16.c)
  endif()

  nuttx_add_application(
    NAME ${PROGNAME}
    PRIORITY ${PRIORITY}
    STACKSIZE ${STACKSIZE}
    MODULE ${MODULE}
    SRCS ${MAINSRC} ${CSRCS}
  )
endif()
