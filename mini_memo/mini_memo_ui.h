/****************************************************************************
 * Copyright (C) 2026 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

#ifndef __MINI_MEMO_UI_H
#define __MINI_MEMO_UI_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MEMO_PAGE_HOME 0
#define MEMO_PAGE_VOICE 1
#define MEMO_PAGE_REVIEW 2
#define MEMO_PAGE_SETTINGS 3

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int memo_ui_init(void);
void memo_ui_deinit(void);
void memo_ui_show_notification(const char* title, const char* body);
void memo_ui_refresh_home(void);
void memo_ui_navigate_to(int page_index);
int memo_ui_start_ptt_selftest(unsigned int hold_ms);

#endif /* __MINI_MEMO_UI_H */
