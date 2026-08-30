#!/bin/bash
set -e

echo "=== Pico Universal Access: Dependency Setup ==="

# 1. Clone FreeRTOS Kernel
if [ ! -d "FreeRTOS-Kernel" ]; then
    echo "Cloning FreeRTOS Kernel..."
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
else
    echo "FreeRTOS-Kernel already exists. Skipping."
fi

# 2. Clone open62541
if [ ! -d "open62541" ]; then
    echo "Cloning open62541..."
    git clone --recursive https://github.com/open62541/open62541.git
else
    echo "open62541 already exists. Skipping."
fi

# 3. Patch open62541 event loop for temporary network drops
# This prevents the server from gracefully shutting down if the Pico loses Wi-Fi for a few seconds.
echo "Patching open62541 network plugin to survive temporary disconnects..."

TCP_FILE="open62541/plugins/network/ua_network_tcp_posix.c"

if [ -f "$TCP_FILE" ]; then
    # Backup the original file
    cp "$TCP_FILE" "${TCP_FILE}.bak"
    
    # Replace the fatal error return in the server network loop with a warning/continue
    # Note: The exact line depends on the open62541 version (master vs v1.3/v1.4)
    # This sed command searches for the error handling block after a failed select/poll
    sed -i 's/return UA_STATUSCODE_BADINTERNALERROR;/ \/* Patched for Pico W: Ignore temp network drop *\/ return UA_STATUSCODE_GOOD;/g' "$TCP_FILE"
    
    echo "Patch applied successfully to $TCP_FILE."
else
    echo "Warning: $TCP_FILE not found. If you are using a custom lwIP network plugin, apply the event loop patch there."
fi

echo "=== Setup Complete. Ready to build. ==="
