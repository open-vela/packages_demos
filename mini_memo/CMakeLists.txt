# ##############################################################################
# packages/demos/mini_memo/CMakeLists.txt
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use
# this file except in compliance with the License. You may obtain a copy of the
# License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.
#
# ##############################################################################

if(CONFIG_LVX_USE_DEMO_MINI_MEMO)
  set(PROGNAME mini_memo)
  set(PRIORITY 100)
  set(STACKSIZE 40960)
  set(MODULE ${CONFIG_LVX_USE_DEMO_MINI_MEMO})
  set(MAINSRC mini_memo_main.c)
  set(CSRCS mini_memo_core.c mini_memo_ui.c)

  nuttx_add_application(
    NAME ${PROGNAME}
    PRIORITY ${PRIORITY}
    STACKSIZE ${STACKSIZE}
    MODULE ${MODULE}
    SRCS ${MAINSRC} ${CSRCS}
  )
endif()
