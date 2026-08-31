# Pico Universal Access

**Industrial OPC UA on a $7 microcontroller.**

An open62541 OPC UA server for the Raspberry Pi Pico 2 W — FreeRTOS, lwIP, and enough headroom for real SCADA clients. Built for edge sensing and light control without proprietary PLC hardware.

---

## Why this exists

Simple data collection and digital I/O should not require expensive controllers. This project puts a standards-based OPC UA endpoint on the Pico 2 W so tools like UaExpert, Ignition, or any compliant client can talk to GPIO, ADC, and onboard diagnostics over Wi-Fi.

---

## Features

- **OPC UA server** — open62541, anonymous access, SecurityPolicy None
- **FreeRTOS + lwIP** — Wi-Fi STA, DHCP, concurrent sessions
- **Stable listen path** — event-loop handling that keeps the server socket alive across transient network errors
- **SNTP time sync** — correct OPC UA timestamps after boot
- **On-device metrics** — CPU temperature, Wi-Fi RSSI, heap usage

**Measured throughput** (single client, continuous read): on the order of **~100 requests/s** in the included benchmark. Concurrent clients are supported (see stress test).

---

## Exposed nodes

| Namespace | Node ID | Access | Description |
|-----------|---------|--------|-------------|
| `ns=1` | `ADC.Channel0` … `ADC.Channel2` | Read | ADC voltage on GPIO 26–28 |
| `ns=1` | `GPIO.0` … `GPIO.22` | Read / Write | Digital GPIO |
| `ns=1` | `Server.CPU_Temperature` | Read | On-chip temperature (°C) |
| `ns=1` | `Server.WiFi_RSSI` | Read | Signal strength (dBm) |
| `ns=1` | `Server.Memory_Allocated` | Read | FreeRTOS heap usage |

**Endpoint (after join):** `opc.tcp://<pico-ip>:4840`

---

## Hardware & tools

| Item | Notes |
|------|--------|
| Board | [Raspberry Pi Pico 2 W](https://www.raspberrypi.com/products/raspberry-pi-pico-2/) |
| SDK | [pico-sdk](https://github.com/raspberrypi/pico-sdk) |
| Host | CMake, ARM GCC (`arm-none-eabi`), git, Linux recommended for the scripts |

---

## Quick start

```bash
git clone https://github.com/srrobicheaux/pico-opcua-server.git
cd pico-opcua-server
chmod +x setup.sh build.sh
./setup.sh -s "YOUR_SSID" -p "YOUR_PASSWORD"
```

`setup.sh` clones FreeRTOS and open62541, applies the event-loop patch, and writes a git-ignored `secrets.h`. Safe to re-run; a full re-run refreshes dependencies from GitHub.

```bash
./build.sh
```

Put the Pico 2 W into **BOOTSEL**, then flash (the script targets `/dev/ttyACM0` by default). First serial output can take up to ~30 seconds while Wi-Fi associates.

```bash
cat /dev/ttyACM0
```

Example:

```text
=== Pico Universal Access ===
wifi_task: Connecting to 'SSID' ... (result=0 link=3 ip=192.168.1.104)
wifi_task: Wi-Fi OK.
sntp_task: Setting time via SNTP ...
SNTP synced to (UTC): Mon Aug 31 02:13:52 2026
opc_task: OPC Server Starting... online.
```

---

## Connect a client

| Setting | Value |
|---------|--------|
| Endpoint | `opc.tcp://<ip>:4840` |
| Security | None / None |
| User | Anonymous |

Optional port check:

```bash
nmap -p 4840 <ip>
```

---

## Tests included

**Single read**

```bash
./opcua-cli.sh read opc.tcp://192.168.1.104:4840 "ns=1;s=ADC.Channel0"
```

**Throughput**

```bash
./readrate.sh -e opc.tcp://192.168.1.104
```

**Multi-client stress**

```bash
./stress-test.sh -c 10 -e 192.168.1.104:4840
```

Scripts lean on a lightweight OPC UA CLI; for day-to-day work, UaExpert or your preferred client is fine.

---

## Project layout (high level)

| Path / script | Role |
|---------------|------|
| `setup.sh` | Dependencies, patch, `secrets.h` |
| `build.sh` | Configure, build, optional flash |
| Application sources | Wi-Fi, SNTP, OPC task, node map |
| open62541 (patched) | Server stack + resilient TCP listen handling |

---

## License & contributing

Open for the community: fork, adapt, and use in the field. Pull requests welcome.

If this displaces a rack of proprietary gear for a simple job, that is the point.

---

**Repository:** [github.com/srrobicheaux/pico-opcua-server](https://github.com/srrobicheaux/pico-opcua-server)
```