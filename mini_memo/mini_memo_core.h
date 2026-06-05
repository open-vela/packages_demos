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

#ifndef __MINI_MEMO_CORE_H
#define __MINI_MEMO_CORE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef enum {
    MEMO_TYPE_MEMO = 0,
    MEMO_TYPE_TODO = 1,
    MEMO_TYPE_SCHEDULE = 2,
} memo_type_t;

typedef struct {
    uint32_t id;
    memo_type_t type;
    char content[200];
    int64_t timestamp;
    int64_t remind_at;
    bool is_read;
} memo_item_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int memo_store_init(const char* data_dir);
void memo_store_deinit(void);
int memo_store_load(void);
int memo_store_save(void);
void memo_store_flush(void);
int memo_store_add(const memo_item_t* item);
int memo_store_delete(uint32_t id);
int memo_store_mark_read(uint32_t id);
int memo_store_get_count(memo_type_t type, bool unread_only);
int memo_store_get_all(memo_item_t* out, int max_items);
int memo_store_get_recent(memo_item_t* out, int max_items);
int memo_store_get_due_reminders(int64_t now, memo_item_t* out, int max_out);
void memo_store_clear_all(void);

/* -- Local Intent Classification ---------------------- */

memo_type_t memo_classify_local(const char* text);

/* -- Voice + AI Integration --------------------------- */

typedef struct {
    memo_type_t type;
    char content[200];
    int64_t remind_at;
} classify_result_t;

/**
 * Initialize VelaClaw client connection.
 * Must be called before memo_voice_* or memo_ai_classify.
 */
int memo_agent_init(void);
void memo_agent_deinit(void);
bool memo_agent_is_connected(void);

/**
 * Start voice recording via voice_channel.
 * Returns 0 on success, negative errno on failure.
 */
int memo_voice_start(void);

/**
 * Stop recording and get ASR transcription.
 * text_out: buffer to receive transcribed text.
 * text_cap: buffer capacity.
 * Returns 0 on success, negative errno on failure.
 */
int memo_voice_stop(char* text_out, size_t text_cap);

/**
 * Classify text using VelaClaw LLM (async).
 * Falls back to memo_classify_local if agent unavailable.
 */
typedef void (*memo_classify_cb)(int status,
    const classify_result_t* result, void* cookie);
int memo_classify_async(const char* text, memo_classify_cb cb, void* cookie);

/**
 * Classify text synchronously (uses LLM if available, else local).
 * Returns 0 on success.
 */
int memo_classify_sync(const char* text, classify_result_t* result);

#endif /* __MINI_MEMO_CORE_H */
