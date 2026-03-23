# Weather Dashboard

\[ English | [简体中文](README_zh-cn.md) \]

## Overview

A feature-rich weather dashboard application built with LVGL for the OpenVela platform. It demonstrates multi-page navigation, real-time data visualization, animated weather effects, and interactive settings — all running on NuttX RTOS.

## Features

- **Multi-page navigation** with animated page transitions (slide + fade)
- **Home page**: Current weather with animated effects (rain/snow/sun glow), 24-hour temperature & humidity line chart, 7-day forecast
- **Detail page**: Circular gauge widgets for temperature, humidity, pressure, and AQI; UV index bar; wind speed & direction display
- **Settings page**: Temperature unit toggle (°C/°F), city selection dropdown, refresh interval slider, about section
- **Simulated sensor data** with realistic fluctuations for demo purposes
- **Dark-themed UI** with a consistent color palette
- **Bottom navigation bar** with icon + label buttons
- **Status bar** showing city name and clock

## Architecture

```
weather_dashboard/
├── CMakeLists.txt                  # CMake build
├── Kconfig                         # NuttX config options
├── Make.defs                       # NuttX build integration
├── Makefile                        # NuttX Makefile
├── README.md                       # English docs
├── README_zh-cn.md                 # Chinese docs
├── weather_dashboard_main.c        # App entry, navigation, timers
└── src/
    ├── weather_data.h              # Data model definitions
    ├── weather_data.c              # Simulated weather data engine
    ├── weather_ui.h                # UI definitions, color palette, page API
    ├── page_home.c                 # Home page (overview + chart + forecast)
    ├── page_detail.c               # Detail page (gauges + UV + wind)
    ├── page_settings.c             # Settings page (unit/city/refresh)
    ├── chart_widget.c              # Reusable 24h line chart widget
    ├── gauge_widget.c              # Reusable circular arc gauge widget
    └── weather_anim.c              # Weather condition particle animations
```

## Design Patterns

- **MVC-like separation**: `weather_data` (Model), `page_*` (View), `weather_dashboard_main` (Controller)
- **Page lifecycle**: Each page implements `create()`, `update()`, `destroy()` callbacks
- **Reusable widgets**: `chart_widget` and `gauge_widget` are self-contained components with private data via `lv_obj_set_user_data()`
- **Animation system**: Particle-based weather effects (rain drops, snowflakes, sun glow) with LVGL animation API

## Build

Enable in NuttX configuration:

```
CONFIG_LVX_USE_DEMO_WEATHER_DASHBOARD=y
```

Optional settings:
```
CONFIG_WEATHER_DASHBOARD_CITY_NAME="Beijing"
CONFIG_WEATHER_DASHBOARD_TEMP_UNIT=0    # 0=Celsius, 1=Fahrenheit
```

## Usage

```shell
nsh> weather_dashboard
```

- Swipe between pages using the bottom navigation bar
- Home: View current weather, hourly chart, and weekly forecast
- Detail: Monitor environment gauges in real-time
- Settings: Toggle units, change city, adjust refresh rate

## Dependencies

- LVGL (v9.x) with chart support
- libuv
- NuttX RTOS

## License

Apache License 2.0
