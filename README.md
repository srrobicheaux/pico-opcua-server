# Pico Universal Access (OPC UA Server)

**Democratizing Industrial Automation**
Why spend thousands of dollars on heavy, proprietary PLCs for simple edge data collection when you can embed an industrial-grade OPC UA server on a $7 Raspberry Pi Pico W? 

This project provides a highly optimized, FreeRTOS-backed OPC UA server running on the RP2040. It is designed to bridge the gap between low-cost microcontrollers and enterprise SCADA/HMI systems. **Steal this with pride** and build amazing industrial integrations without the enterprise price tag.

***You are 5 commands away froma function OPC UA Server.***

## 🚀 Features

* **Real-Time OPC UA Server:** Built on the robust `open62541` stack.
* **Resilient Event Loop:** Custom patched network layer to survive temporary Wi-Fi drops without shutting down the server.
* **High Throughput:** Optimized RTOS task scheduling achieves maximum requests-per-second (req/sec) on lwIP. 
Max 10 concurrent connections. 
Approximately 500ms per request (roundtrip). 
Approximately 20 requests per second throughput.
* **Auto-Provisioning SNTP:** Automatically pulls network time on boot for accurate OPC UA timestamps.
* **Modular Codebase:** Cleanly separated hardware abstraction and OPC handlers for easy scaling.

## 📡 Exposed Endpoints
Once connected to your Wi-Fi network, the Pico exposes the following Node IDs to any standard OPC UA Client (like UaExpert or Ignition):

| Namespace | Node ID | Access | Description |
| :--- | :--- | :--- | :--- |
| `ns=1` | `ADC.Channel0` ... `2` | Read | 12-bit ADC Voltage conversions (GPIO 26-29) |
| `ns=1` | `GPIO.0` ... `22` | Read/Write | Digital state of standard GPIO pins |
| `ns=1` | `Server.CPU_Temperature` | Read | Internal RP2350 CPU temperature (°C) |
| `ns=1` | `Server.WiFi_RSSI` | Read | Live Wi-Fi signal strength (dBm) |
| `ns=1` | `Server.Memory_Allocated`| Read | FreeRTOS heap memory utilization |

## 🛠️ Hardware & Prerequisites

* **Hardware:** [Raspberry Pi Pico 2W](https://www.digikey.com/en/products/detail/raspberry-pi/SC1633/25862726)
* **SDK:** [Raspberry Pi Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk)
* **Build Tools:** CMake, ARM GCC Toolchain, git


## ⚙️ 1. Installation & Build

```bash
git clone https://github.com/srrobicheaux/pico-opcua-server.git
cd pico-opcua-server
chmod +x ./setup.sh
./setup.sh
```

### ./setup.sh -s [WIFI_SSID] -p [WIFI_Password]
   ./setup.sh should be safe to run multiple times. However, it probably only needs to be run once. If it is rerun, it assumes that you want to start fresh with the dependencies and reclones them from github.

   This script pulls the FreeRTOS and open62541 dependencies and applies a custom patch to the open62541 event loop to prevent the server from exiting during transient Wi-Fi drops.

   It also creates a secrets.h file for WiFi credentials.
   *(This file is git-ignored for your security.)*

   It will prompt to build once completed.

## 2. Build & Flash
### ./build.sh
***Caution:*** *build.sh will often remove and recreate the ./build directory used by cmake.*

```bash Build the project
./build.sh
```

   There is at least **500k** lines of code that will compile. Be patient. If the build fails for some reason, you can rerun ./build.sh 

### Flash
Hold the BOOTSEL button on your Pico 2W, plug it into your USB port. The build script assumes the flash will happen on **/dev/ttyACM0**.
*(Potentialy do this while the build process continues as it will eventually attempt a flash)* 

*Once flashed, it may take up to **30** seconds to display anything!*

   I errored on startup speed. As such, I wait for wifi connectivity before printing anything as this is often the slowest process.

   Example startup output from 
```bash
   cat /dev/ttyACM0
```
   > ===	Pico Universal Access	===
   > WiFi  
   > SNTP
   > OPC UAS
   > 
   > wifi_task:	 Connecting to 'SSID' ... (result=0 link=3 ip=192.168.1.104) 
   > wifi_task:	 Wi-Fi OK. 
   > sntp_task:	 Setting time via SNTP ... 
   > SNTP synced to (UTC): Mon Aug 31 02:13:52 2026
   > opc_task:	 OPC Server Starting... online.


## 3. Testing
The project contains multiple bash scripts for testing the robustness of your opcserver. Many of these tools are based on *'opcua-cli-linux-x86_64' paired down for less dependencies.*
*I would highly suggest using this tool instead of my scripts.*

   ### ⚡ Connectivity Testing
   *Although not part of the repo, nmap -p 4840 [Your_IP_Address] will let you know if the opcserver is responding.*

```bash
      opcua-cli.sh 
```
   Example:
      './opcua-cli.sh read opc.tcp://192.168.1.104:4840 "ns=1;s=ADC.Channel0"'
      
      Requested session timeout to be 600000ms, got 10000ms instead
      0.567326009273529

   ### ⚡ Performance Testing
   This repository includes a readrate.sh (throughput) script to benchmark the capabilities of the Pico W's lwIP stack under heavy OPC UA polling. 

   *Note: We reduced the FreeRTOS task delay in the main OPC loop to 1ms to significantly boost transaction speeds over the standard 10ms FreeRTOS tick.*
```bash
   readrate.sh -e [your_IP_address]
```

   Example:
   './readrate.sh -e opc.tcp://192.168.1.104'
   > === Starting OPC UA Throughput Test ===
   > Target: opc.tcp://192.168.1.104:4840
   > Duration: 10 seconds
   > 
   > Connecting to opc.tcp://192.168.1.104:4840...
   > Requested session timeout to be 600000ms, got 10000ms instead
   > Connected! Polling Server.WiFi_RSSI and ADC Channel 0...
   > 
   > --- Benchmark Results ---
   > Total Requests: 1096
   > Duration: 10.00 seconds
   > Throughput: 109.60 requests/sec

   ### Stress Testing
   Also included is a full-blown, multi-threaded dashboard benchmark script with ASCII gauges and live metrics tracking!

```bash
   ./stress-test.sh -e [your_IP_address]
```
   > Usage: ./readrate.sh [options].
   > Options:
   >   -c, --clients <num>    Number of concurrent parallel clients (default: 4)
   >   -v, --verbose          Enable verbose response output from workers
   >   -e, --endpoint <url>   OPC UA endpoint IP:Port (default: 192.168.1.104:4840)
   >   -n, --node <node_id>   Target Node ID (default: ns=1;s=ADC.Channel0)

   Example output from:
```bash
   './stress-test.sh -c 10 -e 192.168.1.104:4840'
```
   >=================================================================
   >        OPC UA Multi-Client Parallel Benchmark Tool               
   >=================================================================
   >  Target Endpoint : opc.tcp://192.168.1.104:4840                 
   >  Target Node ID  : ns=1;s=ADC.Channel0                     
   >  Active Clients  : 10 parallel workers
   >-----------------------------------------------------------------
   >  Throughput      : [░░░░░░░░░░░░░░░░░░░░]    0.0 req/sec
   >  Avg Latency     : 1559 ms / read
   >  Total Requests  : 156    (OK: 156   | ERR: 0    )
   >-----------------------------------------------------------------
   >  Press [Q] or Ctrl+C to stop...
   > 
   > Benchmark stopped. 

🤝 Contributing & License
This project is built for the community. Fork it, improve it, use it in your factories, and submit pull requests.

If you use this to replace a $5,000 piece of equipment, let me know!
EOF