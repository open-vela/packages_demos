电子沙漏 Demo (Hourglass Demo)
项目简介
这是一个基于openvela系统，在模拟器上运行的电子沙漏演示程序。它使用LVGL图形库创建了一个可视化的沙漏效果，用户可以通过触摸屏控制沙漏的启动和计时时间。
功能特性
逼真的沙漏动画：模拟沙粒从上至下的流动效果。
交互控制：
开始/重置：启动或重置沙漏计时。
时间调整：增加或减少沙漏的总计时时间。
模块化设计：UI界面与控制逻辑分离，便于维护和扩展。
前提条件：
已搭建OpenVela开发环境（参考：[环境搭建](https://gitee.com/open-vela/docs/blob/dev/zh-cn/quickstart/Set_up_the_development_environment_zh-cn.md)）
已获取OpenVela源码（参考：[下载openvela源码](https://gitee.com/open-vela/docs/blob/dev/zh-cn/quickstart/Download_Vela_sources_zh-cn.md)）
快速开始
1. 配置项目
### 打开功能选项
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ distclean -j$(nproc) menuconfig
```
在打开的配置界面中，进行如下操作：
按下 / 键，搜索 LVX_USE_DEMO_HOURGLASS。
按回车键进入该配置项。
按下空格键，将选项标记为 [*] (即 =y) 以启用该Demo。
多次按 Q 键退出，并在提示是否保存时按 Y 键。
注意：其他相关配置（如设备路径、任务优先级等）已预设好，无需修改，除非有特殊需求。
2. 编译与构建
在源码根目录下，依次执行以下命令：
### 构建
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ -j$(nproc)
```

### 清理构建产物
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ distclean -j$(nproc)
```

3. 运行Demo
### 启动模拟器
```bash
./emulator.sh vela
```

### 启动计算器应用
```bash
hourglass &
```

项目结构
text
vela/packages/demos/hourglass/
├── hourglass_page.c/h    # UI界面绘制、按钮事件处理
├── hourglass_control.c/h # 沙漏动画的核心控制逻辑与状态机
├── hourglass_main.c      # 程序入口，任务初始化
├── Kconfig               # 项目配置选项（使能、优先级、设备路径等）
├── Make.defs             # 编译系统依赖项定义
└── Makefile              # 编译规则
核心实现说明
UI界面
沙漏主体：由两个旋转45度的正方形容器构成，每个容器内动态创建8x8的红色小方块矩阵来模拟沙粒。
控制面板：包含Start按钮和+/-按钮，用于控制沙漏运行和调整时间。
控制逻辑
状态机：管理沙漏的初始、运行、结束三种状态。
定时器驱动：核心动画由LVGL定时器周期性触发。
沙粒移动算法：使用对角线扫描与随机方向策略，每次移动一粒“沙粒”（即改变一个小方块的颜色），模拟自然下落。
故障排除
Demo未启动：请确认配置步骤中 LVX_USE_DEMO_HOURGLASS 已正确设置为 y。
触摸无响应：检查 LVX_USE_DEMO_HOURGLASS_DEVPATH 配置的设备路径是否与系统实际设备节点匹配。
编译错误：确保开发环境已正确搭建，并执行过彻底的 distclean。

---









