# LoRa Water System Monitoring & Control (ESP32)

A distributed **industrial-style IoT system** for monitoring and controlling a water purification / distribution setup using **two ESP32 nodes communicating over LoRa**.

The system consists of:

* **Sensor Node (ESP32-A)** – reads sensors and sends telemetry
* **Controller Node (ESP32-B)** – user interface + remote control using a DWIN HMI display

The nodes communicate via **LoRa E32 modules** using a lightweight **VP/VAL command protocol**.

---

# System Architecture

                                ┌───────────────────────────────┐
                                │        CONTROLLER NODE        │
                                │           ESP32-B             │
                                │                               │
                                │  • DWIN HMI Interface         │
                                │  • Button Control Logic       │
                                │  • Telemetry Display          │
                                │  • Command Transmission       │
                                └───────────────┬───────────────┘
                                                │
                                                │ UART
                                                │
                                        ┌───────▼───────┐
                                        │   LoRa E32    │
                                        │  Address = 1  │
                                        └───────┬───────┘
                                                │
                           ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ │ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
                               LoRa Wireless Link (868 MHz)
                           ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ │ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
                                                │
                                        ┌───────▼───────┐
                                        │   LoRa E32    │
                                        │  Address = 2  │
                                        └───────┬───────┘
                                                │
                                                │ UART
                                                │
                                ┌───────────────▼───────────────┐
                                │          SENSOR NODE          │
                                │           ESP32-A             │
                                │                               │
                                │  • Sensor acquisition         │
                                │  • Telemetry processing       │
                                │  • LoRa communication         │
                                │  • FreeRTOS multitasking      │
                                └───────────────────────────────┘
# Sensor Node Architecture (ESP32-A)
    +-----------------------------------------------------------+
    |                       SENSOR NODE                         |
    |                         ESP32-A                           |
    |                                                           |
    |  +-----------------------------------------------------+  |
    |  |                   FreeRTOS Tasks                    |  |
    |  |                                                     |  |
    |  |  sensorReadTask()                                   |  |
    |  |   |- readWaterQuality()                             |  |
    |  |   |- readPressure()                                 |  |
    |  |   |- readWaterLevelMM()                             |  |
    |  |   |- readTemperature()                              |  |
    |  |                                                     |  |
    |  |  flowTask()                                         |  |
    |  |   |- pulse counting                                 |  |
    |  |   |- flow rate calculation                          |  |
    |  |                                                     |  |
    |  |  loraRxTask()                                       |  |
    |  |   |- receive LoRa commands                          |  |
    |  |                                                     |  |
    |  |  processTelemetry()                                 |  |
    |  |   |- build telemetry packet                         |  |
    |  +-----------------------------------------------------+  |
    |                                                           |
    |  Shared Data                                              |
    |                                                           |
    |   qualityByte       -> scaled water quality               |
    |   flowByte          -> scaled flow rate                   |
    |   pressureByte      -> scaled pressure                    |
    |   waterLevel        -> tank level                         |
    |   temperatureValue  -> water temperature                  |
    |                                                           |
    +-----------------------------------------------------------+
  # Sensor Hardware Layer
                     +---------------------+
                     |       ESP32-A       |
                     |     Sensor Node     |
                     +----------+----------+
                                |
        +-----------------------+-----------------------+
        |                       |                       |
        v                       v                       v

    ADS1115 ADC            Flow Sensor              DS18B20
       (I2C)               (GPIO Interrupt)         (1-Wire)

     +--------------+      +--------------+        +--------------+
     | Pressure     |      | Water Flow   |        | Temperature  |
     | Sensor       |      | Sensor       |        | Sensor       |
     +--------------+      +--------------+        +--------------+
  
  
              |
              v
  
          TDS Sensor
          (Analog)
  
              |
              v
  
    Ultrasonic Level Sensor
---
# Features

### Sensor Node

* Flow measurement
* Water pressure monitoring
* Water quality measurement (TDS)
* Temperature measurement (DS18B20)
* Ultrasonic water level monitoring
* LoRa telemetry transmission
* FreeRTOS task-based architecture

### Controller Node

* DWIN HMI display interface
* Button command handling
* LoRa command transmission
* Telemetry visualization
* Heartbeat / watchdog communication

---

# Repository Structure

```
lora-water-system
│
├── sensor-node
│   ├── src
│   ├── include
│   └── lib
│
├── controller-node
│   ├── src
│   ├── include
│   └── lib
│
└── README.md
```

Each node contains its own modular firmware architecture.

---

# Sensor Node Structure

```
sensor-node
│
├── src
│   └── main.cpp
│
├── include
│   ├── config.h
│   ├── protocol.h
│   └── shared_data.h
│
└── lib
    ├── sensors
    ├── flow_sensor
    ├── lora_comm
    └── failsafe
```

### Responsibilities

| Module      | Purpose                                 |
| ----------- | --------------------------------------- |
| sensors     | TDS, temperature, pressure, water level |
| flow_sensor | pulse counting + flow calculation       |
| lora_comm   | LoRa communication                      |
| failsafe    | communication timeout safety            |

---

# Controller Node Structure

```
controller-node
│
├── src
│   └── main.cpp
│
├── include
│   ├── config.h
│   ├── protocol.h
│   └── shared_data.h
│
└── lib
    ├── lora_comm
    ├── dwin_display
    ├── button_manager
    └── heartbeat
```

### Responsibilities

| Module         | Purpose                        |
| -------------- | ------------------------------ |
| lora_comm      | communication with sensor node |
| dwin_display   | UI display updates             |
| button_manager | command handling               |
| heartbeat      | system health monitoring       |

---

# Hardware Components

### Microcontrollers

* ESP32 DevKit

### Communication

* EBYTE E32 LoRa module

### Sensors

* Flow sensor (YF-S201 or similar)
* TDS sensor
* Pressure sensor
* DS18B20 temperature sensor
* Ultrasonic water level sensor

### UI

* DWIN HMI display

### I/O Expansion

* PCF8575 I/O expander

### ADC

* ADS1115 external ADC

---

# Library Dependencies

The firmware depends on the following libraries:

* PCF8575 library by Renzo Mischianti
* Adafruit ADS1X15
* OneWire
* DallasTemperature

---

# PlatformIO Setup (Recommended)

Install **PlatformIO** and build using:

```
platformio run
```

Example `platformio.ini`:

```
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200

lib_deps =
    renzo-mischianti/PCF8575 library
    adafruit/Adafruit ADS1X15
    paulstoffregen/OneWire
    milesburton/DallasTemperature
```

---

# Communication Protocol

Nodes exchange compact **VP/VAL messages**.

Example payload:

```
6364
```

Meaning:

```
VP = 0x63  → TDS
VAL = 0x64 → sensor value
```

### Example Telemetry Packet

```
63XX 64XX 61XX 72XX 62XX
```

| VP | Sensor        |
| -- | ------------- |
| 63 | Water quality |
| 64 | Temperature   |
| 61 | Flow rate     |
| 72 | Water level   |
| 62 | Pressure      |

---

# FreeRTOS Tasks

### Sensor Node

| Task           | Purpose        |
| -------------- | -------------- |
| sensorReadTask | reads sensors  |
| flowTask       | pulse counting |
| loraTask       | LoRa receive   |
| telemetry      | send data      |

### Controller Node

| Task       | Purpose           |
| ---------- | ----------------- |
| loraTask   | receive telemetry |
| buttonTask | UI commands       |
| heartbeat  | system monitoring |

---

# Build & Flash

### Sensor Node

```
cd sensor-node
platformio run --target upload
```

### Controller Node

```
cd controller-node
platformio run --target upload
```

---

# Example Telemetry Output

```
WL=53mm Q=120 T=24C F=8.4L/min P=3.2bar
```

---

# Safety Features

* communication timeout failsafe
* watchdog heartbeat
* constrained sensor scaling
* modular firmware architecture

---

# Use Cases

* water purification systems
* industrial tank monitoring
* remote pump stations
* agricultural irrigation systems
* chemical mixing plants

---

# Future Improvements

* MQTT / cloud integration
* OTA firmware updates
* data logging
* web dashboard
* encryption for LoRa communication

---

# Author

Embedded systems developer focused on:

* robotics
* embedded firmware
* autonomous systems
* IoT monitoring systems

Technologies used:

* ESP32
* C++
* FreeRTOS
* LoRa communication
* sensor integration

---

# License

MIT License
