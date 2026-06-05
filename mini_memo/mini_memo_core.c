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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <netutils/cJSON.h>

#include "mini_memo_core.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MEMO_TAG "mini_memo"
#define MEMO_MAX_ITEMS 100
#define MEMO_FILENAME "memos.json"
#define MEMO_TMP_FILENAME "memos.json.tmp"
#define MEMO_PATH_MAX 256
#define MEMO_JSON_VERSION 1

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct {
    memo_item_t items[MEMO_MAX_ITEMS];
    int count;
    uint32_t next_id;
    char data_dir[MEMO_PATH_MAX];
    char file_path[MEMO_PATH_MAX];
    char tmp_path[MEMO_PATH_MAX];
    pthread_mutex_t lock;
    bool initialized;
    bool dirty;
} memo_store_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static memo_store_t g_store;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int ensure_directory(const char* path)
{
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        syslog(LOG_WARNING, "%s: path exists but is not a directory: %s\n",
            MEMO_TAG, path);
        return -ENOTDIR;
    }

    if (mkdir(path, 0755) != 0) {
        int err = errno;
        syslog(LOG_WARNING, "%s: failed to create directory %s: %d\n",
            MEMO_TAG, path, err);
        return -err;
    }

    syslog(LOG_INFO, "%s: created data directory: %s\n", MEMO_TAG, path);
    return 0;
}

static int read_file_contents(const char* path, char** out_buf,
    size_t* out_len)
{
    struct stat st;
    int fd;
    ssize_t nread;
    char* buf;

    if (stat(path, &st) != 0) {
        return -errno;
    }

    if (st.st_size == 0 || st.st_size > (1024 * 1024)) {
        return -EINVAL;
    }

    buf = malloc(st.st_size + 1);
    if (!buf) {
        return -ENOMEM;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        free(buf);
        buf = NULL;
        return -errno;
    }

    nread = read(fd, buf, st.st_size);
    close(fd);

    if (nread != st.st_size) {
        free(buf);
        buf = NULL;
        return -EIO;
    }

    buf[st.st_size] = '\0';
    *out_buf = buf;
    *out_len = st.st_size;
    return 0;
}

static int find_item_index(uint32_t id)
{
    int i;

    for (i = 0; i < g_store.count; i++) {
        if (g_store.items[i].id == id) {
            return i;
        }
    }

    return -1;
}

static void remove_item_at(int index)
{
    if (index < 0 || index >= g_store.count) {
        return;
    }

    if (index < g_store.count - 1) {
        memmove(&g_store.items[index], &g_store.items[index + 1],
            (g_store.count - index - 1) * sizeof(memo_item_t));
    }

    g_store.count--;
}

static void evict_one_item(void)
{
    int oldest_read_idx = -1;
    int64_t oldest_read_ts = INT64_MAX;
    int oldest_idx = 0;
    int64_t oldest_ts = INT64_MAX;
    int i;

    for (i = 0; i < g_store.count; i++) {
        if (g_store.items[i].timestamp < oldest_ts) {
            oldest_ts = g_store.items[i].timestamp;
            oldest_idx = i;
        }

        if (g_store.items[i].is_read && g_store.items[i].timestamp < oldest_read_ts) {
            oldest_read_ts = g_store.items[i].timestamp;
            oldest_read_idx = i;
        }
    }

    if (oldest_read_idx >= 0) {
        remove_item_at(oldest_read_idx);
    } else {
        remove_item_at(oldest_idx);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int memo_store_init(const char* data_dir)
{
    int ret;
    int n;

    if (g_store.initialized) {
        syslog(LOG_WARNING, "%s: store already initialized\n", MEMO_TAG);
        return 0;
    }

    memset(&g_store, 0, sizeof(g_store));

    /* Store data directory path */

    strncpy(g_store.data_dir, data_dir, sizeof(g_store.data_dir) - 1);
    g_store.data_dir[sizeof(g_store.data_dir) - 1] = '\0';

    /* Construct file paths */

    n = snprintf(g_store.file_path, sizeof(g_store.file_path),
        "%s/%s", data_dir, MEMO_FILENAME);
    if (n < 0 || (size_t)n >= sizeof(g_store.file_path)) {
        syslog(LOG_ERR, "%s: file path too long\n", MEMO_TAG);
        return -ENAMETOOLONG;
    }

    n = snprintf(g_store.tmp_path, sizeof(g_store.tmp_path),
        "%s/%s", data_dir, MEMO_TMP_FILENAME);
    if (n < 0 || (size_t)n >= sizeof(g_store.tmp_path)) {
        syslog(LOG_ERR, "%s: tmp path too long\n", MEMO_TAG);
        return -ENAMETOOLONG;
    }

    /* Ensure data directory exists */

    ret = ensure_directory(data_dir);
    if (ret != 0) {
        syslog(LOG_ERR, "%s: cannot ensure data dir: %d\n", MEMO_TAG, ret);
        return ret;
    }

    /* Initialize mutex */

    ret = pthread_mutex_init(&g_store.lock, NULL);
    if (ret != 0) {
        syslog(LOG_ERR, "%s: mutex init failed: %d\n", MEMO_TAG, ret);
        return -ret;
    }

    /* Initialize store state */

    g_store.count = 0;
    g_store.next_id = 1;
    g_store.initialized = true;

    /* Load existing data */

    ret = memo_store_load();
    if (ret != 0) {
        syslog(LOG_WARNING, "%s: load returned %d, starting empty\n",
            MEMO_TAG, ret);
    }

    syslog(LOG_INFO, "%s: initialized, %d items loaded\n",
        MEMO_TAG, g_store.count);
    return 0;
}

void memo_store_deinit(void)
{
    if (!g_store.initialized) {
        return;
    }

    /* Flush pending changes */

    memo_store_save();

    pthread_mutex_destroy(&g_store.lock);
    g_store.initialized = false;

    syslog(LOG_INFO, "%s: deinitialized\n", MEMO_TAG);
}

int memo_store_load(void)
{
    char* buf = NULL;
    size_t buf_len = 0;
    cJSON* root = NULL;
    cJSON* version_obj = NULL;
    cJSON* next_id_obj = NULL;
    cJSON* items_arr = NULL;
    cJSON* item_obj = NULL;
    cJSON* field = NULL;
    int ret;
    int i;
    int arr_size;

    if (!g_store.initialized) {
        return -EINVAL;
    }

    ret = read_file_contents(g_store.file_path, &buf, &buf_len);
    if (ret != 0) {
        /* File doesn't exist or can't be read - start empty */
        syslog(LOG_INFO, "%s: no data file, starting empty\n", MEMO_TAG);
        return 0;
    }

    root = cJSON_Parse(buf);
    free(buf);
    buf = NULL;

    if (!root) {
        syslog(LOG_WARNING, "%s: JSON parse failed, starting empty\n",
            MEMO_TAG);
        return 0;
    }

    /* Validate version */

    version_obj = cJSON_GetObjectItem(root, "version");
    if (!version_obj || !cJSON_IsNumber(version_obj)) {
        syslog(LOG_WARNING, "%s: missing version field\n", MEMO_TAG);
        cJSON_Delete(root);
        root = NULL;
        return 0;
    }

    if (version_obj->valueint != MEMO_JSON_VERSION) {
        syslog(LOG_WARNING, "%s: unsupported version %d\n",
            MEMO_TAG, version_obj->valueint);
        cJSON_Delete(root);
        root = NULL;
        return 0;
    }

    /* Read next_id */

    next_id_obj = cJSON_GetObjectItem(root, "next_id");
    if (next_id_obj && cJSON_IsNumber(next_id_obj)) {
        g_store.next_id = (uint32_t)next_id_obj->valuedouble;
    }

    /* Read items array */

    items_arr = cJSON_GetObjectItem(root, "items");
    if (!items_arr || !cJSON_IsArray(items_arr)) {
        syslog(LOG_WARNING, "%s: missing items array\n", MEMO_TAG);
        cJSON_Delete(root);
        root = NULL;
        return 0;
    }

    arr_size = cJSON_GetArraySize(items_arr);
    if (arr_size > MEMO_MAX_ITEMS) {
        arr_size = MEMO_MAX_ITEMS;
    }

    pthread_mutex_lock(&g_store.lock);
    g_store.count = 0;

    for (i = 0; i < arr_size; i++) {
        item_obj = cJSON_GetArrayItem(items_arr, i);
        if (!item_obj || !cJSON_IsObject(item_obj)) {
            continue;
        }

        memo_item_t* item = &g_store.items[g_store.count];
        memset(item, 0, sizeof(memo_item_t));

        field = cJSON_GetObjectItem(item_obj, "id");
        if (field && cJSON_IsNumber(field)) {
            item->id = (uint32_t)field->valuedouble;
        }

        field = cJSON_GetObjectItem(item_obj, "type");
        if (field && cJSON_IsNumber(field)) {
            item->type = (memo_type_t)field->valueint;
        }

        field = cJSON_GetObjectItem(item_obj, "content");
        if (field && cJSON_IsString(field) && field->valuestring) {
            strncpy(item->content, field->valuestring,
                sizeof(item->content) - 1);
            item->content[sizeof(item->content) - 1] = '\0';
        }

        field = cJSON_GetObjectItem(item_obj, "timestamp");
        if (field && cJSON_IsNumber(field)) {
            item->timestamp = (int64_t)field->valuedouble;
        }

        field = cJSON_GetObjectItem(item_obj, "remind_at");
        if (field && cJSON_IsNumber(field)) {
            item->remind_at = (int64_t)field->valuedouble;
        }

        field = cJSON_GetObjectItem(item_obj, "is_read");
        if (field) {
            item->is_read = cJSON_IsTrue(field) ? true : false;
        }

        g_store.count++;
    }

    pthread_mutex_unlock(&g_store.lock);

    cJSON_Delete(root);
    root = NULL;

    syslog(LOG_INFO, "%s: loaded %d items from disk\n",
        MEMO_TAG, g_store.count);
    return 0;
}

int memo_store_save(void)
{
    cJSON* root = NULL;
    cJSON* items_arr = NULL;
    cJSON* item_obj = NULL;
    char* json_str = NULL;
    int fd;
    ssize_t written;
    size_t json_len;
    int ret = 0;
    int i;

    if (!g_store.initialized) {
        return -EINVAL;
    }

    root = cJSON_CreateObject();
    if (!root) {
        syslog(LOG_ERR, "%s: failed to create JSON object\n", MEMO_TAG);
        return -ENOMEM;
    }

    if (!cJSON_AddNumberToObject(root, "version", MEMO_JSON_VERSION)) {
        cJSON_Delete(root);
        root = NULL;
        return -ENOMEM;
    }

    pthread_mutex_lock(&g_store.lock);

    if (!cJSON_AddNumberToObject(root, "next_id",
            (double)g_store.next_id)) {
        pthread_mutex_unlock(&g_store.lock);
        cJSON_Delete(root);
        root = NULL;
        return -ENOMEM;
    }

    items_arr = cJSON_AddArrayToObject(root, "items");
    if (!items_arr) {
        pthread_mutex_unlock(&g_store.lock);
        cJSON_Delete(root);
        root = NULL;
        return -ENOMEM;
    }

    for (i = 0; i < g_store.count; i++) {
        item_obj = cJSON_CreateObject();
        if (!item_obj) {
            pthread_mutex_unlock(&g_store.lock);
            cJSON_Delete(root);
            root = NULL;
            return -ENOMEM;
        }

        cJSON_AddNumberToObject(item_obj, "id",
            (double)g_store.items[i].id);
        cJSON_AddNumberToObject(item_obj, "type",
            (double)g_store.items[i].type);
        cJSON_AddStringToObject(item_obj, "content",
            g_store.items[i].content);
        cJSON_AddNumberToObject(item_obj, "timestamp",
            (double)g_store.items[i].timestamp);
        cJSON_AddNumberToObject(item_obj, "remind_at",
            (double)g_store.items[i].remind_at);
        cJSON_AddBoolToObject(item_obj, "is_read",
            g_store.items[i].is_read);

        cJSON_AddItemToArray(items_arr, item_obj);
    }

    pthread_mutex_unlock(&g_store.lock);

    /* Serialize to string */

    json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    root = NULL;

    if (!json_str) {
        syslog(LOG_ERR, "%s: failed to serialize JSON\n", MEMO_TAG);
        return -ENOMEM;
    }

    json_len = strlen(json_str);

    /* Atomic write: write to .tmp then rename */

    fd = open(g_store.tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        syslog(LOG_ERR, "%s: failed to open tmp file: %d\n",
            MEMO_TAG, errno);
        free(json_str);
        json_str = NULL;
        return -errno;
    }

    written = write(fd, json_str, json_len);
    close(fd);
    free(json_str);
    json_str = NULL;

    if (written < 0 || (size_t)written != json_len) {
        syslog(LOG_ERR, "%s: write failed\n", MEMO_TAG);
        unlink(g_store.tmp_path);
        return -EIO;
    }

    /* Rename atomically */

    ret = rename(g_store.tmp_path, g_store.file_path);
    if (ret != 0) {
        ret = -errno;
        syslog(LOG_ERR, "%s: rename failed: %d\n", MEMO_TAG, -ret);
        unlink(g_store.tmp_path);
        return ret;
    }

    return 0;
}

int memo_store_add(const memo_item_t* item)
{
    memo_item_t* new_item;

    if (!g_store.initialized || !item) {
        return -EINVAL;
    }

    pthread_mutex_lock(&g_store.lock);

    /* Enforce capacity: evict if full */

    if (g_store.count >= MEMO_MAX_ITEMS) {
        evict_one_item();
    }

    /* Add new item */

    new_item = &g_store.items[g_store.count];
    memcpy(new_item, item, sizeof(memo_item_t));
    new_item->id = g_store.next_id++;

    /* Ensure content is null-terminated */

    new_item->content[sizeof(new_item->content) - 1] = '\0';

    g_store.count++;

    pthread_mutex_unlock(&g_store.lock);

    /* Mark dirty - will be flushed by periodic timer */

    g_store.dirty = true;

    return (int)new_item->id;
}

int memo_store_delete(uint32_t id)
{
    int index;

    if (!g_store.initialized) {
        return -EINVAL;
    }

    pthread_mutex_lock(&g_store.lock);

    index = find_item_index(id);
    if (index < 0) {
        pthread_mutex_unlock(&g_store.lock);
        return -ENOENT;
    }

    remove_item_at(index);

    pthread_mutex_unlock(&g_store.lock);

    g_store.dirty = true;
    return 0;
}

int memo_store_mark_read(uint32_t id)
{
    int index;

    if (!g_store.initialized) {
        return -EINVAL;
    }

    pthread_mutex_lock(&g_store.lock);

    index = find_item_index(id);
    if (index < 0) {
        pthread_mutex_unlock(&g_store.lock);
        return -ENOENT;
    }

    g_store.items[index].is_read = true;

    pthread_mutex_unlock(&g_store.lock);

    g_store.dirty = true;
    return 0;
}

int memo_store_get_count(memo_type_t type, bool unread_only)
{
    int count = 0;
    int i;

    if (!g_store.initialized) {
        return 0;
    }

    pthread_mutex_lock(&g_store.lock);

    for (i = 0; i < g_store.count; i++) {
        if (g_store.items[i].type != type) {
            continue;
        }

        if (unread_only && g_store.items[i].is_read) {
            continue;
        }

        count++;
    }

    pthread_mutex_unlock(&g_store.lock);

    return count;
}

int memo_store_get_all(memo_item_t* out, int max_items)
{
    int copy_count;

    if (!g_store.initialized || !out || max_items <= 0) {
        return 0;
    }

    pthread_mutex_lock(&g_store.lock);
    copy_count = g_store.count < max_items ? g_store.count : max_items;
    memcpy(out, g_store.items, copy_count * sizeof(memo_item_t));
    pthread_mutex_unlock(&g_store.lock);

    return copy_count;
}

int memo_store_get_recent(memo_item_t* out, int max_items)
{
    int start;
    int copy_count;

    if (!g_store.initialized || !out || max_items <= 0) {
        return 0;
    }

    pthread_mutex_lock(&g_store.lock);
    copy_count = g_store.count < max_items ? g_store.count : max_items;
    start = g_store.count - copy_count;
    memcpy(out, &g_store.items[start], copy_count * sizeof(memo_item_t));
    pthread_mutex_unlock(&g_store.lock);

    return copy_count;
}

int memo_store_get_due_reminders(int64_t now, memo_item_t* out, int max_out)
{
    int n = 0;
    int i;

    if (!g_store.initialized || !out || max_out <= 0) {
        return 0;
    }

    pthread_mutex_lock(&g_store.lock);
    for (i = 0; i < g_store.count && n < max_out; i++) {
        if (g_store.items[i].remind_at > 0 && g_store.items[i].remind_at <= now && !g_store.items[i].is_read) {
            out[n++] = g_store.items[i];
        }
    }
    pthread_mutex_unlock(&g_store.lock);

    return n;
}

void memo_store_clear_all(void)
{
    if (!g_store.initialized) {
        return;
    }

    pthread_mutex_lock(&g_store.lock);
    g_store.count = 0;
    pthread_mutex_unlock(&g_store.lock);

    g_store.dirty = true;

    syslog(LOG_INFO, "%s: all items cleared\n", MEMO_TAG);
}

void memo_store_flush(void)
{
    if (!g_store.initialized || !g_store.dirty) {
        return;
    }

    g_store.dirty = false;
    memo_store_save();
}

/****************************************************************************
 * memo_classify_local - Offline keyword-based intent classification
 *
 * This function provides a local fallback for intent classification when
 * the VelaClaw LLM service is unavailable. It uses simple substring
 * matching to classify text into memo types.
 *
 * Input:
 *   text - UTF-8 text to classify (may be NULL)
 *
 * Returns:
 *   MEMO_TYPE_TODO     - if todo keywords found
 *   MEMO_TYPE_SCHEDULE - if schedule keywords found
 *   MEMO_TYPE_MEMO     - default (no keyword match or NULL input)
 ****************************************************************************/

memo_type_t memo_classify_local(const char* text)
{
    /* Handle NULL input gracefully */

    if (!text) {
        syslog(LOG_INFO, "%s: classify_local: NULL input, default MEMO\n",
            MEMO_TAG);
        return MEMO_TYPE_MEMO;
    }

    /* Check for TODO keywords */

    if (strstr(text, "\xe6\x8f\x90\xe9\x86\x92\xe6\x88\x91") != NULL || /* ti xing wo (remind me) */
        strstr(text, "\xe5\x88\xab\xe5\xbf\x98\xe4\xba\x86") != NULL || /* bie wang le (do not forget) */
        strstr(text, "\xe5\xbe\x85\xe5\x8a\x9e") != NULL || /* dai ban (todo) */
        strstr(text, "todo") != NULL) {
        syslog(LOG_INFO, "%s: classify_local: matched TODO\n", MEMO_TAG);
        return MEMO_TYPE_TODO;
    }

    /* Check for SCHEDULE keywords */

    if (strstr(text, "\xe5\xae\x89\xe6\x8e\x92") != NULL || /* an pai (arrange) */
        strstr(text, "\xe7\xba\xa6\xe4\xba\x86") != NULL || /* yue le (appointment) */
        strstr(text, "\xe5\x87\xa0\xe7\x82\xb9") != NULL || /* ji dian (what time) */
        strstr(text, "\xe6\x97\xa5\xe7\xa8\x8b") != NULL || /* ri cheng (schedule) */
        strstr(text, "schedule") != NULL) {
        syslog(LOG_INFO, "%s: classify_local: matched SCHEDULE\n",
            MEMO_TAG);
        return MEMO_TYPE_SCHEDULE;
    }

    /* Default: MEMO */

    syslog(LOG_INFO, "%s: classify_local: no keyword match, default MEMO\n",
        MEMO_TAG);
    return MEMO_TYPE_MEMO;
}

/****************************************************************************
 * Voice + AI Agent Integration
 ****************************************************************************/

#include <velaclaw/client.h>

/* voice_channel is an internal velaclaw module; we link against it
 * since mini_memo depends on CONFIG_EXAMPLES_AI_AGENT_VELA */

extern int voice_channel_init(void);
extern int voice_channel_start(void);
extern int voice_channel_stop_with_text(char* text_out, size_t text_cap);
extern void voice_channel_cleanup(void);

/****************************************************************************
 * Private Data - Agent
 ****************************************************************************/

static velaclaw_client_t* g_client;
static bool g_agent_connected;

/****************************************************************************
 * Private Functions - Agent
 ****************************************************************************/

static int parse_classify_json(const char* json_str,
    classify_result_t* result)
{
    cJSON* root = NULL;
    cJSON* type_obj = NULL;
    cJSON* content_obj = NULL;
    cJSON* remind_obj = NULL;
    const char* type_str;

    if (!json_str || !result) {
        return -EINVAL;
    }

    root = cJSON_Parse(json_str);
    if (!root) {
        syslog(LOG_WARNING, "%s: classify JSON parse failed\n", MEMO_TAG);
        return -EINVAL;
    }

    type_obj = cJSON_GetObjectItem(root, "type");
    content_obj = cJSON_GetObjectItem(root, "content");
    remind_obj = cJSON_GetObjectItem(root, "remind_at");

    if (!type_obj || !cJSON_IsString(type_obj)) {
        cJSON_Delete(root);
        return -EINVAL;
    }

    type_str = type_obj->valuestring;
    if (strcmp(type_str, "todo") == 0) {
        result->type = MEMO_TYPE_TODO;
    } else if (strcmp(type_str, "schedule") == 0) {
        result->type = MEMO_TYPE_SCHEDULE;
    } else {
        result->type = MEMO_TYPE_MEMO;
    }

    if (content_obj && cJSON_IsString(content_obj)) {
        strncpy(result->content, content_obj->valuestring,
            sizeof(result->content) - 1);
        result->content[sizeof(result->content) - 1] = '\0';
    } else {
        result->content[0] = '\0';
    }

    if (remind_obj && cJSON_IsNumber(remind_obj)) {
        result->remind_at = (int64_t)remind_obj->valuedouble;
    } else {
        result->remind_at = 0;
    }

    cJSON_Delete(root);
    return 0;
}

static const char* g_classify_prompt_fmt = "You are a memo classifier. Given the user's voice input, "
                                           "classify it and extract structured data.\n\n"
                                           "Input: \"%s\"\n\n"
                                           "Respond ONLY with JSON:\n"
                                           "{\"type\":\"memo|todo|schedule\","
                                           "\"content\":\"<cleaned content>\","
                                           "\"remind_at\":<unix_timestamp_or_0>}\n\n"
                                           "Rules:\n"
                                           "- \"memo\": general notes\n"
                                           "- \"todo\": tasks with reminders\n"
                                           "- \"schedule\": appointments with times\n"
                                           "- content: concise version of input\n"
                                           "- remind_at: extract time if mentioned, else 0";

/****************************************************************************
 * Public Functions - Agent
 ****************************************************************************/

int memo_agent_init(void)
{
    int voice_ret;

    syslog(LOG_INFO, "%s: agent init\n", MEMO_TAG);

    /* Voice is local to this process and should remain usable even if the
     * optional VelaClaw client cannot connect. */

    voice_ret = voice_channel_init();
    if (voice_ret < 0) {
        syslog(LOG_WARNING, "%s: voice_channel_init failed: %d\n",
            MEMO_TAG, voice_ret);
    }

    /* Open VelaClaw client for remote LLM classification. This is optional:
     * the app already falls back to local classification when disconnected. */

    g_client = velaclaw_client_open("mini_memo");
    if (!g_client) {
        syslog(LOG_WARNING, "%s: velaclaw_client_open failed\n", MEMO_TAG);
        g_agent_connected = false;
        return voice_ret;
    }

    g_agent_connected = true;
    syslog(LOG_INFO, "%s: agent connected\n", MEMO_TAG);
    return voice_ret;
}

void memo_agent_deinit(void)
{
    syslog(LOG_INFO, "%s: agent deinit\n", MEMO_TAG);

    voice_channel_cleanup();

    if (g_client) {
        velaclaw_client_close(g_client);
        g_client = NULL;
    }

    g_agent_connected = false;
}

bool memo_agent_is_connected(void)
{
    return g_agent_connected;
}

int memo_voice_start(void)
{
    int ret;

    syslog(LOG_INFO, "%s: voice_start\n", MEMO_TAG);
    ret = voice_channel_start();
    if (ret < 0) {
        syslog(LOG_ERR, "%s: voice_channel_start failed: %d\n",
            MEMO_TAG, ret);
    }

    return ret;
}

int memo_voice_stop(char* text_out, size_t text_cap)
{
    int ret;

    if (!text_out || text_cap == 0) {
        return -EINVAL;
    }

    syslog(LOG_INFO, "%s: voice_stop\n", MEMO_TAG);
    ret = voice_channel_stop_with_text(text_out, text_cap);
    if (ret < 0) {
        syslog(LOG_ERR, "%s: voice_channel_stop_with_text failed: %d\n",
            MEMO_TAG, ret);
        text_out[0] = '\0';
    } else {
        syslog(LOG_INFO, "%s: ASR result: \"%s\"\n", MEMO_TAG, text_out);
    }

    return ret;
}

/* Callback context for async classification */

typedef struct {
    memo_classify_cb user_cb;
    void* user_cookie;
    char input_text[200];
} classify_ctx_t;

static void classify_response_cb(int status, const char* response_json,
    void* cookie)
{
    classify_ctx_t* ctx = (classify_ctx_t*)cookie;
    classify_result_t result;
    int ret;

    memset(&result, 0, sizeof(result));

    if (status != 0 || !response_json) {
        /* LLM failed, fall back to local classification */

        syslog(LOG_WARNING, "%s: LLM classify failed (status=%d), "
                            "using local\n",
            MEMO_TAG, status);
        result.type = memo_classify_local(ctx->input_text);
        strncpy(result.content, ctx->input_text,
            sizeof(result.content) - 1);
        result.content[sizeof(result.content) - 1] = '\0';
        result.remind_at = 0;
    } else {
        ret = parse_classify_json(response_json, &result);
        if (ret < 0) {
            /* Parse failed, fall back to local */

            syslog(LOG_WARNING, "%s: classify JSON parse failed, "
                                "using local\n",
                MEMO_TAG);
            result.type = memo_classify_local(ctx->input_text);
            strncpy(result.content, ctx->input_text,
                sizeof(result.content) - 1);
            result.content[sizeof(result.content) - 1] = '\0';
            result.remind_at = 0;
        }
    }

    if (ctx->user_cb) {
        ctx->user_cb(0, &result, ctx->user_cookie);
    }

    free(ctx);
}

int memo_classify_async(const char* text, memo_classify_cb cb, void* cookie)
{
    classify_ctx_t* ctx = NULL;
    velaclaw_ask_req_t req;
    char* prompt = NULL;
    int prompt_len;
    int ret;

    if (!text || !cb) {
        return -EINVAL;
    }

    /* If agent not connected, do local classification immediately */

    if (!g_agent_connected || !g_client) {
        classify_result_t result;

        memset(&result, 0, sizeof(result));
        result.type = memo_classify_local(text);
        strncpy(result.content, text, sizeof(result.content) - 1);
        result.content[sizeof(result.content) - 1] = '\0';
        result.remind_at = 0;
        cb(0, &result, cookie);
        return 0;
    }

    /* Build prompt */

    prompt_len = strlen(g_classify_prompt_fmt) + strlen(text) + 16;
    prompt = malloc(prompt_len);
    if (!prompt) {
        return -ENOMEM;
    }

    snprintf(prompt, prompt_len, g_classify_prompt_fmt, text);

    /* Allocate callback context */

    ctx = malloc(sizeof(classify_ctx_t));
    if (!ctx) {
        free(prompt);
        return -ENOMEM;
    }

    ctx->user_cb = cb;
    ctx->user_cookie = cookie;
    strncpy(ctx->input_text, text, sizeof(ctx->input_text) - 1);
    ctx->input_text[sizeof(ctx->input_text) - 1] = '\0';

    /* Send to VelaClaw */

    memset(&req, 0, sizeof(req));
    req.text = prompt;
    req.timeout_ms = 15000;

    ret = velaclaw_ask(g_client, &req, classify_response_cb, ctx);
    free(prompt);
    prompt = NULL;

    if (ret < 0) {
        syslog(LOG_ERR, "%s: velaclaw_ask failed: %d\n", MEMO_TAG, ret);
        free(ctx);
        ctx = NULL;

        /* Fall back to local */

        classify_result_t result;

        memset(&result, 0, sizeof(result));
        result.type = memo_classify_local(text);
        strncpy(result.content, text, sizeof(result.content) - 1);
        result.content[sizeof(result.content) - 1] = '\0';
        result.remind_at = 0;
        cb(0, &result, cookie);
        return 0;
    }

    return 0;
}

/* Sync wrapper context */

typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    bool done;
    int status;
    classify_result_t result;
} sync_classify_ctx_t;

static void sync_classify_cb(int status,
    const classify_result_t* result, void* cookie)
{
    sync_classify_ctx_t* ctx = (sync_classify_ctx_t*)cookie;

    pthread_mutex_lock(&ctx->mtx);
    ctx->status = status;
    if (status == 0 && result) {
        ctx->result = *result;
    }
    ctx->done = true;
    pthread_cond_signal(&ctx->cond);
    pthread_mutex_unlock(&ctx->mtx);
}

int memo_classify_sync(const char* text, classify_result_t* result)
{
    sync_classify_ctx_t ctx;
    struct timespec ts;
    int ret;

    if (!text || !result) {
        return -EINVAL;
    }

    memset(result, 0, sizeof(*result));

    /* If agent not connected, use local directly */

    if (!g_agent_connected || !g_client) {
        result->type = memo_classify_local(text);
        strncpy(result->content, text, sizeof(result->content) - 1);
        result->content[sizeof(result->content) - 1] = '\0';
        result->remind_at = 0;
        return 0;
    }

    /* Use async path with sync wait */

    pthread_mutex_init(&ctx.mtx, NULL);
    pthread_cond_init(&ctx.cond, NULL);
    ctx.done = false;
    ctx.status = -1;
    memset(&ctx.result, 0, sizeof(ctx.result));

    ret = memo_classify_async(text, sync_classify_cb, &ctx);
    if (ret < 0) {
        pthread_mutex_destroy(&ctx.mtx);
        pthread_cond_destroy(&ctx.cond);
        return ret;
    }

    /* Wait up to 20 seconds */

    pthread_mutex_lock(&ctx.mtx);
    if (!ctx.done) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 20;
        pthread_cond_timedwait(&ctx.cond, &ctx.mtx, &ts);
    }
    pthread_mutex_unlock(&ctx.mtx);

    if (ctx.done && ctx.status == 0) {
        *result = ctx.result;
    } else {
        /* Timeout or error - fallback to local */

        result->type = memo_classify_local(text);
        strncpy(result->content, text, sizeof(result->content) - 1);
        result->content[sizeof(result->content) - 1] = '\0';
        result->remind_at = 0;
    }

    pthread_mutex_destroy(&ctx.mtx);
    pthread_cond_destroy(&ctx.cond);
    return 0;
}
