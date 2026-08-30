#!/bin/bash

# Default to local loopback or pass Pico IP as an argument
TARGET_IP=${1:-"192.168.1.50"}
URL="opc.tcp://${TARGET_IP}:4840"
DURATION=10

# Use the virtual environment python interpreter if it exists
PYTHON_BIN="./.venv/bin/python"
if [ ! -f "$PYTHON_BIN" ]; then
    PYTHON_BIN="python3"
fi

echo "=== Starting OPC UA Throughput Test ==="
echo "Target: $URL"
echo "Duration: ${DURATION} seconds"
echo ""

"$PYTHON_BIN" - <<EOF
import time
import asyncio
from asyncua import Client

async def run_benchmark():
    url = "$URL"
    client = Client(url=url)
    print(f"Connecting to {url}...")
    try:
        await client.connect()
        print("Connected! Polling Server.WiFi_RSSI and ADC Channel 0...")
        
        node_rssi = client.get_node("ns=1;s=Server.WiFi_RSSI")
        node_adc0 = client.get_node("ns=1;s=ADC.Channel0")
        
        count = 0
        start_time = time.time()
        end_time = start_time + $DURATION
        
        while time.time() < end_time:
            await node_rssi.get_value()
            await node_adc0.get_value()
            count += 2  
            
        elapsed = time.time() - start_time
        req_sec = count / elapsed
        
        print("\n--- Benchmark Results ---")
        print(f"Total Requests: {count}")
        print(f"Duration: {elapsed:.2f} seconds")
        print(f"Throughput: {req_sec:.2f} requests/sec")
        
        await client.disconnect()
    except Exception as e:
        print(f"\nConnection or polling failed: {e}")
        print("Make sure your Pico W is powered on, connected to Wi-Fi, and the IP address is correct.")

asyncio.run(run_benchmark())
EOF