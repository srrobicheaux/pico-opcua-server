# Pico Universal Access (OPC UA Server)

**Democratizing Industrial Automation.** 

Why spend thousands of dollars on heavy, proprietary PLCs for simple edge data collection when you can embed an industrial-grade OPC UA server on a $7 Raspberry Pi Pico W? 

This project provides a highly optimized, FreeRTOS-backed OPC UA server running on the RP2040. It is designed to bridge the gap between low-cost microcontrollers and enterprise SCADA/HMI systems. **Steal this with pride** and build amazing industrial integrations without the enterprise price tag.

## 🚀 Features

* **Real-Time OPC UA Server:** Built on the robust `open62541` stack.
* **Resilient Event Loop:** Custom patched network layer to survive temporary Wi-Fi drops without shutting down the server.
* **High Throughput:** Optimized RTOS task scheduling achieves maximum requests-per-second (req/sec) on lwIP.
* **Auto-Provisioning SNTP:** Automatically pulls network time on boot for accurate OPC UA timestamps.
* **Modular Codebase:** Cleanly separated hardware abstraction and OPC handlers for easy scaling.

## 📡 Exposed Endpoints

Once connected to your Wi-Fi network, the Pico exposes the following Node IDs to any standard OPC UA Client (like UaExpert or Ignition):

| Namespace | Node ID | Access | Description |
| :--- | :--- | :--- | :--- |
| `ns=1` | `ADC.Channel0` ... `3` | Read | 12-bit ADC Voltage conversions (GPIO 26-29) |
| `ns=1` | `GPIO.0` ... `22` | Read/Write | Digital state of standard GPIO pins |
| `ns=1` | `Server.CPU_Temperature` | Read | Internal RP2040 CPU temperature (°C) |
| `ns=1` | `Server.WiFi_RSSI` | Read | Live Wi-Fi signal strength (dBm) |
| `ns=1` | `Server.Memory_Allocated`| Read | FreeRTOS heap memory utilization |

## 🛠️ Hardware & Prerequisites

* **Hardware:** Raspberry Pi Pico W
* **SDK:** [Raspberry Pi Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk)
* **Build Tools:** CMake, ARM GCC Toolchain

## ⚙️ Installation & Setup

1. **Clone this repository:**
   ```bash
   git clone [https://github.com/srrobicheaux/pico-opcua-server.git](https://github.com/srrobicheaux/pico-opcua-server.git)
   cd pico-opcua-server

   Run the setup script:
This script pulls the FreeRTOS and open62541 dependencies and applies a custom patch to the open62541 event loop to prevent the server from exiting during transient Wi-Fi drops.

Bash
chmod +x setup.sh
./setup.sh
Configure Wi-Fi Secrets:
Create a secrets.h file in your src directory (this file is git-ignored for your security):

C
#ifndef SECRETS_H
#define SECRETS_H
#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_Password"
#endif
Build the Firmware:

Bash
mkdir build && cd build
cmake ..
make -j4
Flash: Hold the BOOTSEL button on your Pico W, plug it into your USB port, and drag the resulting .uf2 file onto the mounted drive.

⚡ Performance Testing
This repository includes a readrate.sh (throughput) script to benchmark the capabilities of the Pico W's lwIP stack under heavy OPC UA polling.

To test your req/sec throughput, run:

Bash
./readrate.sh <PICO_IP_ADDRESS>
Note: We reduced the FreeRTOS task delay in the main OPC loop to 1ms to significantly boost transaction speeds over the standard 10ms FreeRTOS tick.

🤝 Contributing & License
This project is built for the community. Fork it, improve it, use it in your factories, and submit pull requests.

If you use this to replace a $5,000 piece of equipment, let me know!
EOF