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

#include "ai_log.h"
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
        buffer->tail_index = ((buffer->tail_index + 1) & AI_RING_BUFFER_MASK(buffer));

    /* Place data in buffer */
    buffer->buffer[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & AI_RING_BUFFER_MASK(buffer));
}

void ai_circular_buffer_queue_arr(ai_circular_buffer_t* buffer, const char* data, ai_circular_buffer_size_t size)
{
    ai_circular_buffer_size_t i;
    for (i = 0; i < size; i++)
        ai_circular_buffer_queue(buffer, data[i]);
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
    if (ai_circular_buffer_is_empty(buffer))
        return 0;

    char* data_ptr = &buffer->drop_char;
    ai_circular_buffer_size_t cnt = 0;
    while ((cnt < len) && ai_circular_buffer_dequeue(buffer, data_ptr)) {
        cnt++;
    }

    return cnt;
}

ai_circular_buffer_size_t ai_circular_buffer_dequeue_arr(ai_circular_buffer_t* buffer, char* data, ai_circular_buffer_size_t len)
{
    if (ai_circular_buffer_is_empty(buffer))
        return 0;

    char* data_ptr = data;
    ai_circular_buffer_size_t cnt = 0;
    while ((cnt < len) && ai_circular_buffer_dequeue(buffer, data_ptr)) {
        cnt++;
        data_ptr++;
    }

    return cnt;
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