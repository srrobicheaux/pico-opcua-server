#!/bin/bash
set -e

echo "=== Pico Universal Access: Full Environment Setup ==="

# 1. Clone FreeRTOS Kernel 
if [ -d "FreeRTOS-Kernel" ]; then
  rmdir FreeRTOS-Kernel
fi
    echo "Cloning FreeRTOS Kernel..."
    git clone --recursive https://github.com/FreeRTOS/FreeRTOS-Kernel.git --depth 1

# 2. Clone open62541 
if [ -d "open62541" ]; then
  rmdir open62541
fi
    echo "Cloning open62541..."
    git clone --recursive https://github.com/open62541/open62541.git --depth 1

# 3. Locate and copy pico_sdk_import.cmake from the Pico SDK environment
if [ -n "$PICO_SDK_PATH" ]; then
    if [ -f "$PICO_SDK_PATH/external/pico_sdk_import.cmake" ]; then
        echo "Copying pico_sdk_import.cmake from PICO_SDK_PATH..."
        cp "$PICO_SDK_PATH/external/pico_sdk_import.cmake" ./
    else
        echo "Warning: pico_sdk_import.cmake not found in \$PICO_SDK_PATH/external/"
    fi
else
    echo "Warning: PICO_SDK_PATH environment variable is not set."
fi

# 4. Locate and copy FreeRTOS_Kernel_import.cmake from the FreeRTOS-Kernel tree
FREERTOS_IMPORT_SRC=$(find FreeRTOS-Kernel -name "FreeRTOS_Kernel_import.cmake" | head -n 1)
if [ -n "$FREERTOS_IMPORT_SRC" ]; then
    echo "Found FreeRTOS_Kernel_import.cmake at $FREERTOS_IMPORT_SRC. Copying to root..."
    cp "$FREERTOS_IMPORT_SRC" ./
else
    echo "Error: FreeRTOS_Kernel_import.cmake could not be found inside FreeRTOS-Kernel!"
    exit 1
fi

# 5. Create a Python Virtual Environment for readrate.sh dependencies
echo "Setting up Python virtual environment for readrate.sh..."
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
fi

./.venv/bin/pip install --upgrade pip --quiet
./.venv/bin/pip install opcua-client asyncua --quiet

if [ ! -f "secrets.h" ]; then
    echo "#define WIFI_SSID       \"YourSSID\"" > secrets.h
    echo "#define WIFI_PASSWORD   \"YourWifiPWD\"" >> secrets.h
fi
echo "=== Setup Complete. All dependencies and build hooks are in place. ==="