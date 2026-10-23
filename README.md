# 🚀 M5AtomS3R — Premium Edge Gateway Firmware for ESP32-S3

<div align="center">

![M5AtomS3R](https://img.shields.io/badge/M5AtomS3R-ESP32--S3%20Gateway-0f172a?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus)
![Arduino](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino)
![Prometheus](https://img.shields.io/badge/Observability-Prometheus-E6522C?style=for-the-badge&logo=prometheus)
![Status](https://img.shields.io/badge/Target-M5Stack%20AtomS3R-22c55e?style=for-the-badge)

[![Repository](https://img.shields.io/badge/GitHub-paulocfmarques--collab%2FM5AtomS3R-181717?logo=github)](https://github.com/paulocfmarques-collab/M5AtomS3R)
[![Last Commit](https://img.shields.io/github/last-commit/paulocfmarques-collab/M5AtomS3R)](https://github.com/paulocfmarques-collab/M5AtomS3R/commits/main)
[![Issues](https://img.shields.io/github/issues/paulocfmarques-collab/M5AtomS3R)](https://github.com/paulocfmarques-collab/M5AtomS3R/issues)
[![PRs](https://img.shields.io/github/issues-pr/paulocfmarques-collab/M5AtomS3R)](https://github.com/paulocfmarques-collab/M5AtomS3R/pulls)

</div>

---

## ✨ Overview

**M5AtomS3R** is a production-style firmware for **M5Stack AtomS3R (ESP32-S3)** focused on:

- resilient network gateway behavior,
- serial + UDP command execution,
- local UX (display, button, RGB effects, screensaver with accelerometer),
- OTA update support,
- observability with Prometheus metrics,
- weather data integration (Open-Meteo, Porto Alegre profile),
- operational diagnostics and stability tooling.

Repository: **`paulocfmarques-collab/M5AtomS3R`**  
Default branch: **`main`**  
Primary language: **C++**

---

## 🗺️ Table of Contents

- [System Architecture](#-system-architecture)
- [Runtime Flow](#-runtime-flow)
- [Repository Map](#-repository-map)
- [Feature Matrix](#-feature-matrix)
- [Display & Input UX](#-display--input-ux)
- [Command Channels](#-command-channels)
- [Prometheus & Telemetry](#-prometheus--telemetry)
- [Build & Flash](#-build--flash)
- [Operation Endpoints](#-operation-endpoints)
- [Project Documents](#-project-documents)
- [Notes & Limitations](#-notes--limitations)

---

## 🧱 System Architecture

```mermaid
flowchart TB
    subgraph Device["M5Stack AtomS3R (ESP32-S3)"]
      App["M5AtomS3.ino\nsetup + app loop"]
      CP["CommandProcessor"]
      GW["ESP32Gateway"]
      DISP["DisplayUtil"]
      LED["RGBLed"]
      NTP["NTPUtil"]
      WEATHER["ClimaManager"]
      OTA["OTAManager"]
      NET["NetworkMonitor"]
      MET["MetricsService"]
      SD["SDUtil (stub/no SD HW)"]
      CRASH["CrashDiagnostics"]
      UI["PortalUi"]
      SERIAL["SerialConsole"]
      SPI["SharedSpi"]
    end

    User["Operator"] -->|USB Serial 115200| SERIAL
    User -->|UDP :4210| GW
    SERIAL --> CP
    GW --> CP
    CP --> DISP
    CP --> LED
    CP --> NTP
    CP --> SD
    App --> OTA
    App --> WEATHER
    App --> NET
    CP --> MET
    NET --> MET
    GW --> UI
    SPI --> SD

    Prom["Prometheus"] -->|scrape /metrics| MET
    WeatherAPI["Open-Meteo API"] --> WEATHER
```

---

## 🔄 Runtime Flow

```mermaid
sequenceDiagram
    autonumber
    participant Boot as Boot
    participant App as application(core1)
    participant Weather as weather(core0)
    participant IO as Serial/UDP
    participant Cmd as CommandProcessor
    participant UX as Display/RGB
    participant Obs as Metrics

    Boot->>App: setup() + init modules
    App->>App: start task pinned to core1
    App->>Weather: periodic weather updates
    App->>IO: poll serial + UDP command input
    IO->>Cmd: command string
    Cmd->>Cmd: parse + validate
    Cmd->>UX: visual feedback / page changes
    Cmd->>Obs: counters & state
    Cmd-->>IO: command response
    App->>App: button state machine / screensaver / page cycle
```

---

## 🗂️ Repository Map

### Core firmware files

| File | Purpose |
|---|---|
| `M5AtomS3.ino` | Main boot/setup/task loop, button behavior, page flow, OTA handling |
| `CommandProcessor.h/.cpp` | Command parser, dispatch and command execution logic |
| `ESP32Gateway.h/.cpp` | Wi-Fi gateway behavior, UDP commands, portal client handling |
| `MetricsService.h/.cpp` | Prometheus-style metrics export and counters |
| `NetworkMonitor.h/.cpp` | Link/network diagnostics and monitoring helpers |
| `NTPUtil.h/.cpp` | Time sync, timezone and DST handling |
| `DisplayUtil.h/.cpp` | Screen pages, UI rendering, screensaver face/eyes |
| `RGBLed.h/.cpp` | RGB control/effects via LP5562 channels |
| `OTAManager.h` | OTA initialization and progress handling |
| `ClimaManager.h` | Weather cache/update strategy (Open-Meteo) |
| `CrashDiagnostics.h/.cpp` | Crash and reset diagnostics |
| `PortalUi.h` | Web portal UI and route-level rendering |
| `SerialConsole.h` | Serial line ingestion and command extraction |
| `SDUtil.h/.cpp` | SD API compatibility (board has no built-in microSD reader) |
| `SharedSpi.h` | Shared SPI bus coordination |

### Documentation assets

| File | Description |
|---|---|
| `README.md` | Main project overview (this document) |
| `LEIA-ME.md` | Detailed Portuguese operational guide |
| `comandos.md` | Full command catalog and operational semantics |
| `comandos.pdf` | Command guide in PDF format |
| `PROMETHEUS.md` | Metrics definitions and PromQL examples |
| `PROMETHEUS.pdf` | Prometheus guide in PDF format |
| `changelog.md` | Local change history |

---

## 🧩 Feature Matrix

```mermaid
mindmap
  root((M5AtomS3R))
    Connectivity
      Wi-Fi profiles in NVS
      AP fallback (ATOMS3_CONFIG)
      UDP command listener :4210
      Network monitoring
    Commanding
      Serial console
      UDP command parser
      health and selftest commands
      device info/status commands
    UX
      IPS 128x128 pages
      Button short/long hold actions
      RGB effects
      Eyes + accelerometer screensaver
    Time & Data
      NTP sync
      Manual time set
      Timezone + DST persisted
      Weather cache (Porto Alegre)
    Observability
      /metrics endpoint
      UDP counters
      Weather metrics
      Device identity labels
    Lifecycle
      OTA updates
      Crash diagnostics
      Reset reason tracking
```

---

## 🖥️ Display & Input UX

### Display pages (logical indices)

| Page | Content |
|---|---|
| `0` | Clock + IP + weather temperature |
| `1` and `2` | NTP / date-time / timezone / DST |
| `3` and `4` | Network information |
| `5` | System info, heap/PSRAM, uptime |
| `6` | Saved Wi-Fi profiles |
| `7` | SD unavailable notice |
| `8` | Network monitor |
| `9` | Porto Alegre forecast |

### Button behavior

- **Short press**: next distinct page (skips duplicates and SD placeholder in normal navigation).
- **Hold ~3s and release**: reboot.
- **Hold ~10s and release**: clear Wi-Fi profiles and reboot.
- **Idle ~60s**: eyes screensaver engages.
- **~20s after navigation inactivity**: returns to clock page.

```mermaid
stateDiagram-v2
    [*] --> Clock
    Clock --> NTP: short press
    NTP --> Network: short press
    Network --> System: short press
    System --> SavedWiFi: short press
    SavedWiFi --> Monitor: short press
    Monitor --> Forecast: short press
    Forecast --> Clock: short press

    Clock --> Screensaver: idle 60s
    NTP --> Screensaver: idle 60s
    Network --> Screensaver: idle 60s
    System --> Screensaver: idle 60s
    SavedWiFi --> Screensaver: idle 60s
    Monitor --> Screensaver: idle 60s
    Forecast --> Screensaver: idle 60s
    Screensaver --> Clock: button/command
```

---

## 📨 Command Channels

The firmware processes commands from two equivalent sources:

1. **USB Serial** (115200 baud)
2. **UDP** to device IP on port **4210**

Input limit is **255 bytes** per command.  
Canonical command documentation is in [`comandos.md`](./comandos.md).

### High-level command groups

- system/diagnostic (`info`, `status`, `health`, `selftest`, `reboot`, `crash_info`, ...)
- network/Wi-Fi (`wifi_add`, `wifi_list`, `wifi_status`, `net_info`, `reset_wifi`, ...)
- display/brightness (`tela`, `tela_next`, `rotacao`, `brilho`, `tela_on`, `tela_off`)
- time/NTP (`time`, `date`, `ntp_status`, `set_fuso`, `dst_on`, `dst_off`, `set_time`)
- RGB (`led_on`, `led_off`, `led_rgb`, `led_breath`, `led_pisca`, `led_blink`)
- weather (`clima`, `clima_age`, `previsao`, `clima_sync`)

```mermaid
flowchart LR
    A[Command Input] --> B{Source}
    B -->|Serial| C[SerialConsole]
    B -->|UDP| D[ESP32Gateway]
    C --> E[CommandProcessor]
    D --> E
    E --> F[Subsystem Action]
    F --> G[Display/LED feedback]
    F --> H[Metrics update]
    E --> I[Response to requester]
```

---

## 📡 Prometheus & Telemetry

Detailed reference is in [`PROMETHEUS.md`](./PROMETHEUS.md).

### Metrics families highlighted by the repo docs

- UDP health/counters:
  - `esp_udp_listener_up`
  - `esp_udp_packets_received_total`
  - `esp_udp_replies_sent_total`
  - `esp_udp_send_errors_total`
- Weather/device:
  - `sensor_temperature_celsius{source="open_meteo",location="porto_alegre"}`
  - `sensor_humidity_percent{source="open_meteo",location="porto_alegre"}`
  - `esp_info{model="M5Stack-AtomS3R"}`

```mermaid
flowchart TD
    S1[UDP listener] --> M[MetricsService]
    S2[Command execution] --> M
    S3[Network monitor] --> M
    S4[Weather cache] --> M
    M --> E[/metrics endpoint]
    E --> P[Prometheus scrape]
    P --> D[Dashboards + alerts]
```

---

## 🛠️ Build & Flash

From project docs (`LEIA-ME.md`):

- Board package reference: **esp32 by Espressif Systems 3.3.12**
- Board selection: **M5AtomS3** (M5Unified identifies AtomS3R)
- Flash: **8MB (64Mb)**
- PSRAM: **OPI PSRAM**
- Partitions: **8M with spiffs (3MB APP/1.5MB SPIFFS), OTA-compatible**
- Arduino runs on **Core 1**, events on **Core 0**

```mermaid
flowchart LR
    IDE[Arduino IDE] --> CFG[ESP32-S3 / M5AtomS3 settings]
    CFG --> BUILD[Compile M5AtomS3.ino]
    BUILD --> FLASH[USB flash first install]
    FLASH --> RUN[Boot + Wi-Fi + services]
    RUN --> OTA[Next updates via OTA host AtomS3R-Gateway]
```

---

## 🌐 Operation Endpoints

| Service | Endpoint / Value |
|---|---|
| AP SSID (config mode) | `ATOMS3_CONFIG` |
| AP Wi-Fi config page | `http://192.168.4.1/wifi` |
| Portal root (station mode) | `http://IP_DA_PLACA/` |
| Portal Prometheus settings | `http://IP_DA_PLACA/prometheus` |
| Metrics endpoint | `http://IP_DA_PLACA:9120/metrics` *(default port; configurable)* |
| UDP command socket | `IP_DA_PLACA:4210` |
| OTA hostname | `AtomS3R-Gateway` |

---

## 📚 Project Documents

- [`LEIA-ME.md`](./LEIA-ME.md) — setup details and device behavior notes
- [`comandos.md`](./comandos.md) — full command contract
- [`PROMETHEUS.md`](./PROMETHEUS.md) — metrics and PromQL examples
- [`changelog.md`](./changelog.md) — change history

---

## ⚠️ Notes & Limitations

- AtomS3R has **no built-in microSD reader** in this adaptation; `sd_*` commands return unavailable.
- UDP and web configuration flows are intended for trusted networks.
- OTA password is not described as enabled in docs.
- Weather profile/documentation is currently centered on **Porto Alegre**.
- Main logic is task-driven with explicit multi-core behavior (app core + weather core).

---

## 🤝 Contributing

Contributions are welcome.

1. Fork repository.
2. Create a feature branch.
3. Keep commits focused and descriptive.
4. Open a PR with hardware validation notes when applicable.

---

## 🔗 Repository

- GitHub: https://github.com/paulocfmarques-collab/M5AtomS3R
