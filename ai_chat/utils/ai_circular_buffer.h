/****************************************************************************
 * packages/demos/ai_chat/utils/ai_circular_buffer.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef FRAMEWORKS_AI_RING_BUFFER_H_
#define FRAMEWORKS_AI_RING_BUFFER_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AI_RING_BUFFER_ASSERT(x) assert(x)

#define AI_RING_BUFFER_IS_POWER_OF_TWO(buffer_size) ((buffer_size & (buffer_size - 1)) == 0)

typedef size_t ai_circular_buffer_size_t;

#define AI_RING_BUFFER_MASK(rb) (rb->buffer_mask)

typedef struct ai_circular_buffer_s ai_circular_buffer_t;

struct ai_circular_buffer_s {
    char* buffer;
    ai_circular_buffer_size_t buffer_mask;
    ai_circular_buffer_size_t tail_index;
    ai_circular_buffer_size_t head_index;
    char drop_char;
};

void ai_circular_buffer_init(ai_circular_buffer_t* buffer, char* buf, size_t buf_size);
void ai_circular_buffer_queue(ai_circular_buffer_t* buffer, char data);
void ai_circular_buffer_queue_arr(ai_circular_buffer_t* buffer, const char* data, ai_circular_buffer_size_t size);
uint8_t ai_circular_buffer_dequeue(ai_circular_buffer_t* buffer, char* data);
ai_circular_buffer_size_t ai_circular_buffer_clear_arr(ai_circular_buffer_t* buffer, ai_circular_buffer_size_t len);
ai_circular_buffer_size_t ai_circular_buffer_dequeue_arr(ai_circular_buffer_t* buffer, char* data, ai_circular_buffer_size_t len);
uint8_t ai_circular_buffer_peek(ai_circular_buffer_t* buffer, char* data, ai_circular_buffer_size_t index);
uint8_t ai_circular_buffer_is_empty(ai_circular_buffer_t* buffer);
uint8_t ai_circular_buffer_is_full(ai_circular_buffer_t* buffer);
ai_circular_buffer_size_t ai_circular_buffer_num_items(ai_circular_buffer_t* buffer);

#ifdef __cplusplus
}
#endif

#endif // FRAMEWORKS_AI_RING_BUFFER_H_
