#!/bin/bash

TARGET_IP="192.168.1.104"
DURATION=10

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -e|--endpoint) 
            # Strip opc.tcp:// prefix if passed in full
            RAW_URL="$2"
            TARGET_IP=$(echo "$RAW_URL" | sed 's|opc.tcp://||' | cut -d: -f1)
            shift ;;
        -c|--duration) DURATION="$2"; shift ;;
        *) TARGET_IP="$1" ;;
    esac
    shift
done

URL="opc.tcp://${TARGET_IP}:4840"

echo "=== Starting OPC UA Throughput Test ==="
echo "Target: $URL"
echo "Duration: ${DURATION} seconds"
echo ""

./.venv/bin/python - <<EOF
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

asyncio.run(run_benchmark())
EOF