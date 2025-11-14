/****************************************************************************
 * packages/demos/ai_chat/utils/ai_circular_buffer.c
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

#include <stdlib.h>
#include <string.h>

#include "ai_circular_buffer.h"

void ai_circular_buffer_init(ai_circular_buffer_t* buffer, char* buf, size_t buf_size)
{
    AI_RING_BUFFER_ASSERT(AI_RING_BUFFER_IS_POWER_OF_TWO(buf_size) == 1);
    buffer->buffer = buf;
    buffer->buffer_mask = buf_size - 1;
    buffer->tail_index = 0;
    buffer->head_index = 0;
}

void ai_circular_buffer_queue(ai_circular_buffer_t* buffer, char data)
{
    if (ai_circular_buffer_is_full(buffer))
        return;

    /* Place data in buffer */
    buffer->buffer[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & AI_RING_BUFFER_MASK(buffer));
}

void ai_circular_buffer_queue_arr(ai_circular_buffer_t* buffer, const char* data, ai_circular_buffer_size_t size)
{
    if (!buffer || !buffer->buffer || !data || size == 0) return;

    const ai_circular_buffer_size_t cap = buffer->buffer_mask + 1;
    ai_circular_buffer_size_t used = ((buffer->head_index - buffer->tail_index) & buffer->buffer_mask);
    ai_circular_buffer_size_t free_space = cap - used;

    if (free_space < size) {
        return;
    }

    ai_circular_buffer_size_t head = buffer->head_index & buffer->buffer_mask;
    ai_circular_buffer_size_t first = cap - head;
    if (first > size) first = size;
    ai_circular_buffer_size_t second = size - first;

    memcpy(buffer->buffer + head, data, first);
    if (second > 0) memcpy(buffer->buffer, data + first, second);

    buffer->head_index = (buffer->head_index + size) & buffer->buffer_mask;
}

uint8_t ai_circular_buffer_dequeue(ai_circular_buffer_t* buffer, char* data)
{
    if (ai_circular_buffer_is_empty(buffer))
        return 0;

    *data = buffer->buffer[buffer->tail_index];
    buffer->tail_index = ((buffer->tail_index + 1) & AI_RING_BUFFER_MASK(buffer));

    return 1;
}

ai_circular_buffer_size_t ai_circular_buffer_clear_arr(ai_circular_buffer_t* buffer, ai_circular_buffer_size_t len)
{
    if (!buffer || !buffer->buffer) return 0;

    ai_circular_buffer_size_t avail = ((buffer->head_index - buffer->tail_index) & buffer->buffer_mask);
    if (avail == 0) return 0;

    if (len > avail) len = avail;

    buffer->tail_index = (buffer->tail_index + len) & buffer->buffer_mask;
    return len;
}

ai_circular_buffer_size_t ai_circular_buffer_dequeue_arr(ai_circular_buffer_t* buffer, char* data, ai_circular_buffer_size_t len)
{
    if (!buffer || !buffer->buffer || !data || len == 0) return 0;

    ai_circular_buffer_size_t avail = ((buffer->head_index - buffer->tail_index) & buffer->buffer_mask);
    if (avail == 0) return 0;

    if (len > avail) len = avail;

    const ai_circular_buffer_size_t cap = buffer->buffer_mask + 1;
    ai_circular_buffer_size_t tail = buffer->tail_index & buffer->buffer_mask;

    ai_circular_buffer_size_t first = cap - tail;
    if (first > len) first = len;
    ai_circular_buffer_size_t second = len - first;

    memcpy(data, buffer->buffer + tail, first);
    if (second > 0) memcpy(data + first, buffer->buffer, second);

    buffer->tail_index = (buffer->tail_index + len) & buffer->buffer_mask;
    return len;
}

uint8_t ai_circular_buffer_peek(ai_circular_buffer_t* buffer, char* data, ai_circular_buffer_size_t index)
{
    if (index >= ai_circular_buffer_num_items(buffer))
        return 0;

    /* Add index to pointer */
    ai_circular_buffer_size_t data_index = ((buffer->tail_index + index) & AI_RING_BUFFER_MASK(buffer));
    *data = buffer->buffer[data_index];

    return 1;
}

uint8_t ai_circular_buffer_is_empty(ai_circular_buffer_t* buffer)
{
    return (buffer->head_index == buffer->tail_index);
}

uint8_t ai_circular_buffer_is_full(ai_circular_buffer_t* buffer)
{
    return ((buffer->head_index - buffer->tail_index) & AI_RING_BUFFER_MASK(buffer)) == AI_RING_BUFFER_MASK(buffer);
}

ai_circular_buffer_size_t ai_circular_buffer_num_items(ai_circular_buffer_t* buffer)
{
    return ((buffer->head_index - buffer->tail_index) & AI_RING_BUFFER_MASK(buffer));
}