# AI Chat Demo 插件开发指南
## **1. 概述**

欢迎大家想基于此 demo 开发自己的 ai 服务，可以遵循 `plugin` 接口的标准来自己实现demo

### **1.1 架构说明**

```plain&#x20;text
ai_conversation (核心层)
    ↓
conversation_plugin (插件管理层)
    ↓
volc_plugin / custom_plugin (具体服务实现)
```

### **1.2 核心概念**

\- **插件接口**: \`conversation\_engine\_plugin\_t\` 定义了所有插件必须实现的标准接口

\- **事件回调**: 通过 \`conversation\_engine\_callback\_t\` 向上层传递对话事件

\- **MCP 工具**: 支持函数调用,扩展 AI 能力

***

## **2. 插件接口规范**

### **2.1 插件结构**

```c
typedef struct conversation_engine_plugin_s {
    const char* name;                          // 插件名称
    int priv_size;                             // 私有数据大小
    int (*init)(void* engine, const conversation_engine_init_params_t* param);
    int (*uninit)(void* engine);
    int (*event_cb)(void* engine, conversation_engine_callback_t callback, void* cookie);
    int (*start)(void* engine);
    int (*write_audio)(void* engine, const char* data, int len);
    int (*finish)(void* engine);
    int (*cancel)(void* engine);
    int (*mcp_response)(void* engine, char* result);
    conversation_engine_env_params_t* (*get_env)(void* engine);
} conversation_engine_plugin_t;
```

### **2.2 必须实现的函数**

| 函数             | 说明            | 返回值       |
| -------------- | ------------- | --------- |
| `init`         | 初始化插件,建立连接    | 0 成功,负数失败 |
| `uninit`       | 清理资源,断开连接     | 0 成功,负数失败 |
| `event_cb`     | 设置事件回调函数      | 0 成功,负数失败 |
| `start`        | 开始对话,准备接收音频   | 0 成功,负数失败 |
| `write_audio`  | 写入音频数据        | 0 成功,负数失败 |
| `finish`       | 结束音频输入,等待响应   | 0 成功,负数失败 |
| `cancel`       | 取消当前对话        | 0 成功,负数失败 |
| `mcp_response` | 返回 MCP 工具执行结果 | 0 成功,负数失败 |
| `get_env`      | 获取环境参数        | 环境参数指针    |

***

## **3. 开发步骤**

### **3.1 创建插件目录结构**

```plain&#x20;text
src/conversation/
├── plugin/                    # 插件框架(已存在)
│   ├── ai_conversation_plugin.h
│   ├── ai_conversation_plugin.c
│   └── ai_conversation_defs.h
└── your_service/              # 你的服务插件目录
    ├── ai_your_service_conversation.c
    └── ai_your_service_conversation.h
```

### **3.2 定义插件实例**

```c
conversation_engine_plugin_t your_service_conversation_engine_plugin = {
    .name = "your_service_conversation",
    .priv_size = sizeof(your_service_conversation_engine_t),
    .init = your_service_init,
    .uninit = your_service_uninit,
    .event_cb = your_service_event_cb,
    .start = your_service_start,
    .write_audio = your_service_write_audio,
    .finish = your_service_finish,
    .cancel = your_service_cancel,
    .mcp_response = your_service_mcp_response,
    .get_env = your_service_get_env,
};
```

***

## **4. 事件处理**

### **4.1 事件类型**

```c
typedef enum {
    conversation_engine_event_unknown,
    conversation_engine_event_start,           // 对话开始
    conversation_engine_event_stop,            // 对话停止
    conversation_engine_event_complete,        // 对话完成
    conversation_engine_event_input_text,      // 输入文本
    conversation_engine_event_audio_start,     // 音频开始
    conversation_engine_event_audio,           // 音频数据
    conversation_engine_event_text,            // 文本响应
    conversation_engine_event_mcp_call,        // MCP 工具调用
    conversation_engine_event_mcp_argument,    // MCP 参数
    conversation_engine_event_error,           // 错误事件
} conversation_engine_event_t;
```

### **4.2 触发事件回调**

```c
static void your_service_send_event(your_service_conversation_engine_t* engine,
                                   conversation_engine_event_t event,
                                   const char* result, int len,
                                   conversation_engine_error_t error_code)
{
    if (engine->event_callback) {
        conversation_engine_result_t res = {
            .result = result,
            .len = len,
            .error_code = error_code
        };
        engine->event_callback(event, &res, engine->event_cookie);
    }
}
```

### **4.3 处理服务响应**

当你的服务返回响应时,需要根据响应类型触发相应的事件:

```c
// 处理文本响应
void your_service_handle_text_response(your_service_conversation_engine_t* engine, 
                                       const char* text)
{
    your_service_send_event(engine, conversation_engine_event_text, 
                           text, strlen(text), conversation_engine_error_success);
}

// 处理音频响应
void your_service_handle_audio_response(your_service_conversation_engine_t* engine,
                                        const char* audio_data, int len)
{
    your_service_send_event(engine, conversation_engine_event_audio_start, 
                           NULL, 0, conversation_engine_error_success);
    your_service_send_event(engine, conversation_engine_event_audio, 
                           audio_data, len, conversation_engine_error_success);
}

// 处理 MCP 工具调用
void your_service_handle_mcp_call(your_service_conversation_engine_t* engine,
                                  const char* tool_name, const char* call_id,
                                  const char* arguments)
{
    // 保存工具调用信息
    if (engine->call_tool_name) free(engine->call_tool_name);
    if (engine->call_tool_id) free(engine->call_tool_id);
    
    engine->call_tool_name = strdup(tool_name);
    engine->call_tool_id = strdup(call_id);
    
    // 触发 MCP 调用事件
    char* mcp_data = malloc(strlen(tool_name) + strlen(call_id) + strlen(arguments) + 3);
    sprintf(mcp_data, "%s|%s|%s", tool_name, call_id, arguments);
    
    your_service_send_event(engine, conversation_engine_event_mcp_call,
                           mcp_data, strlen(mcp_data), conversation_engine_error_success);
    
    free(mcp_data);
}
```

***

## **5. 注册插件**

### **5.1 在 ai\_conversation.c 中注册**

```c
// 在 ai_conversation.c 顶部添加
extern conversation_engine_plugin_t your_service_conversation_engine_plugin;

// 在 ai_conversation_create_engine 函数中添加插件选择逻辑
conversation_handle_t ai_conversation_create_engine(const conversation_init_params_t* param)
{
    conversation_engine_plugin_t* plugin = NULL;
    
    switch (param->engine_type) {
        case CONVERSATION_ENGINE_TYPE_VOLC:
            plugin = &volc_conversation_engine_plugin;
            break;
        case CONVERSATION_ENGINE_TYPE_YOUR_SERVICE:  // 新增
            plugin = &your_service_conversation_engine_plugin;
            break;
        default:
            CON_ERR("Unknown engine type: %d", param->engine_type);
            return NULL;
    }
    
    // ... 其余代码
}
```

### **5.2 在 ai\_conversation.h 中添加枚举**

```c
typedef enum {
    CONVERSATION_ENGINE_TYPE_VOLC,
    CONVERSATION_ENGINE_TYPE_YOUR_SERVICE,  // 新增
} conversation_engine_type;
```

***

## **6. 配置与编译**

### **6.1 添加 Kconfig 选项**

```plain&#x20;text
config AI_YOUR_SERVICE_PLUGIN
    bool "Your service plugin"
    depends on AI_CONVERSATION_DEMO
    default n
    help
      Enable your custom AI service plugin

config YOUR_SERVICE_API_KEY
    string "Your service API key"
    depends on AI_YOUR_SERVICE_PLUGIN
    default "YOUR_API_KEY"
    help
      API key for your service

config YOUR_SERVICE_API_URL
    string "Your service API URL"
    depends on AI_YOUR_SERVICE_PLUGIN
    default "https://api.yourservice.com"
    help
      API endpoint URL for your service
```

### **6.2 修改 Makefile**

```makefile
ifeq ($(CONFIG_AI_YOUR_SERVICE_PLUGIN),y)
  CSRCS += conversation/your_service/ai_your_service_conversation.c
endif
```

***

## **7. 参考示例**

完整的火山服务插件实现位于:

* `src/conversation/volc/ai_volc_conversation.c`

* `src/conversation/volc/ai_volc_conversation.h`

建议开发者参考该实现,理解插件接口的使用方式和最佳实践。

***

## **8. 附录**

### **8.1 完整示例代码**

参见 `src/conversation/volc/` 目录下的火山服务插件实现。

### **8.2 接口头文件**

* `include/ai_conversation.h` - 对话引擎公共接口

* `src/conversation/plugin/ai_conversation_plugin.h` - 插件接口定义

* `src/conversation/plugin/ai_conversation_defs.h` - 插件相关类型定义

## **9. 快速开始**

### **9.1 最小化插件模板**

```c
// ai_minimal_conversation.c

#include "ai_conversation_plugin.h"
#include "ai_log.h"

typedef struct {
    conversation_engine_callback_t event_callback;
    void* event_cookie;
    conversation_engine_init_params_t config;
    conversation_engine_env_params_t env;
} minimal_engine_t;

static int minimal_init(void* engine, const conversation_engine_init_params_t* param)
{
    CON_INFO("Minimal plugin initialized");
    return 0;
}

static int minimal_uninit(void* engine)
{
    CON_INFO("Minimal plugin uninitialized");
    return 0;
}

static int minimal_event_cb(void* engine, conversation_engine_callback_t callback, void* cookie)
{
    CON_INFO("Minimal plugin minimal_event_cb");
    return 0;
}

static int minimal_start(void* engine)
{
    CON_INFO("Minimal plugin start");
    return 0;
}

static int minimal_write_audio(void* engine, const char* data, int len)
{
    // 处理音频数据
    return 0;
}

static int minimal_finish(void* engine)
{
    CON_INFO("Minimal plugin finish");
    return 0;
}

static int minimal_cancel(void* engine)
{
    return 0;
}

static int minimal_mcp_response(void* engine, char* result)
{
    return 0;
}

static conversation_engine_env_params_t* minimal_get_env(void* engine)
{
    minimal_engine_t* e = (minimal_engine_t*)engine;
    return &e->env;
}

conversation_engine_plugin_t minimal_conversation_engine_plugin = {
    .name = "minimal_conversation",
    .priv_size = sizeof(minimal_engine_t),
    .init = minimal_init,
    .uninit = minimal_uninit,
    .event_cb = minimal_event_cb,
    .start = minimal_start,
    .write_audio = minimal_write_audio,
    .finish = minimal_finish,
    .cancel = minimal_cancel,
    .mcp_response = minimal_mcp_response,
    .get_env = minimal_get_env,
};
```
