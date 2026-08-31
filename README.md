# Pico Universal Access

**Get started with OPC UA for under $10.**

An industrial-style OPC UA server on the Raspberry Pi Pico 2 W — open62541, FreeRTOS, and Wi-Fi — so you can experiment with real clients (UaExpert, Ignition, custom tools) without a PLC budget.

Hardware cost is on the order of a Pico 2 W board (~$7). Software is open source.

**Repository:** [github.com/srrobicheaux/pico-opcua-server](https://github.com/srrobicheaux/pico-opcua-server)

---

## Why this exists

OPC UA is the standard language of modern SCADA and HMI systems, but trying it often means expensive hardware or heavy PCs. This project puts a working server on a microcontroller you can hold in two fingers: GPIO, ADC, temperature, Wi-Fi signal, and heap stats — all as OPC UA nodes.

Aimed at:

- Controls and OT engineers exploring edge OPC UA
- Students and educators
- OEMs and makers who need a cheap, standards-based endpoint for demos and prototypes

It is **not** a replacement for a plant DCS or safety PLC. It **is** a fast way to learn, prototype, and instrument simple systems.

---

## Features

- **OPC UA server** — open62541, anonymous login, SecurityPolicy None (lab / LAN use)
- **FreeRTOS + lwIP** — Wi-Fi station mode, DHCP, concurrent sessions
- **Resilient listen path** — event-loop handling that keeps the server socket alive across transient network errors
- **SNTP time sync** — sensible OPC UA timestamps after boot
- **On-device metrics** — CPU temperature, Wi-Fi RSSI, FreeRTOS heap usage

**Measured throughput** (included single-client benchmark): on the order of **~100 requests/s**. Multi-client stress scripts are included.

---

## Exposed nodes

| Namespace | Node ID | Access | Description |
|-----------|---------|--------|-------------|
| `ns=1` | `ADC.Channel0` … `ADC.Channel2` | Read | ADC voltage on GPIO 26–28 |
| `ns=1` | `GPIO.0` … `GPIO.22` | Read / Write | Digital GPIO |
| `ns=1` | `Server.CPU_Temperature` | Read | On-chip temperature (°C) |
| `ns=1` | `Server.WiFi_RSSI` | Read | Signal strength (dBm) |
| `ns=1` | `Server.Memory_Allocated` | Read | FreeRTOS heap usage |

**Endpoint after Wi-Fi join:** `opc.tcp://<pico-ip>:4840`

---

## Security notice (read this)

This build is intended for **trusted networks, labs, and demos**:

- Security policy: **None**
- Authentication: **Anonymous**

Do **not** expose it to the public internet or to untrusted plant networks without hardening (encryption, user auth, network isolation). Production OT deployments need proper security design; this project prioritizes learning and accessibility first.

---

## Hardware & tools

| Item | Notes |
|------|--------|
| Board | [Raspberry Pi Pico 2 W](https://www.raspberrypi.com/products/raspberry-pi-pico-2/) (~$7) |
| SDK | [pico-sdk](https://github.com/raspberrypi/pico-sdk) |
| Host | CMake, ARM GCC (`arm-none-eabi`), git — Linux recommended for the helper scripts |

---

## Quick start

```bash
git clone https://github.com/srrobicheaux/pico-opcua-server.git
cd pico-opcua-server
chmod +x setup.sh build.sh
./setup.sh -s "YOUR_SSID" -p "YOUR_PASSWORD"
```

`setup.sh` fetches FreeRTOS and open62541, applies the event-loop patch, and writes a git-ignored `secrets.h`. Re-running is safe; a full re-run refreshes dependencies from GitHub.

```bash
./build.sh
```

Hold **BOOTSEL**, plug in the Pico, and flash (default device `/dev/ttyACM0`). First useful serial output can take up to ~30 seconds while Wi-Fi associates.

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

Optional:

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

For day-to-day browsing, UaExpert or any compliant OPC UA client works well.

---

## Project layout

| Path / script | Role |
|---------------|------|
| `setup.sh` | Dependencies, patch, `secrets.h` |
| `build.sh` | Configure, build, optional flash |
| Application sources | Wi-Fi, SNTP, OPC task, node map |
| open62541 (patched) | Server stack + resilient TCP listen handling |

---

## GitHub discovery tips

If you fork or star related work, these **topics** help others find Pico + OPC UA projects:

`opc-ua` · `open62541` · `raspberry-pi-pico` · `pico-2-w` · `rp2350` · `freertos` · `lwip` · `industrial-iot` · `scada` · `edge-computing`

Add a short **About** description on the repo (e.g. *OPC UA server on Raspberry Pi Pico 2 W*) and a clear **LICENSE** file (MIT is a common choice for this kind of project).

---

## License & contributing

Community project: fork it, adapt it, use it in the lab or on the bench. Pull requests welcome.
[MIT License](./LICENSE)
**Under $10. Real OPC UA. Your network, your nodes.**
