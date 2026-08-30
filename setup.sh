#!/bin/bash
set -e

echo "=== Pico Universal Access: Full Environment Setup ==="

# 1. Clone FreeRTOS Kernel if not present
if [ ! -d "FreeRTOS-Kernel" ]; then
    echo "Cloning FreeRTOS Kernel..."
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git --depth 1
else
    echo "FreeRTOS-Kernel already exists. Skipping."
fi

# 2. Clone open62541 if not present
if [ ! -d "open62541" ]; then
    echo "Cloning open62541..."
    git clone --recursive https://github.com/open62541/open62541.git --depth 1
else
    echo "open62541 already exists. Skipping."
fi

# 3. Locate and copy pico_sdk_import.cmake from the Pico SDK environment
if [ -n "$PICO_SDK_PATH" ]; then
    if [ -f "$PICO_SDK_PATH/external/pico_sdk_import.cmake" ]; then
        echo "Copying pico_sdk_import.cmake from PICO_SDK_PATH..."
        cp "$PICO_SDK_PATH/external/pico_sdk_import.cmake" ./
    else
        echo "Warning: pico_sdk_import.cmake not found in \$PICO_SDK_PATH/external/"
    fi
else
    echo "Warning: PICO_SDK_PATH environment variable is not set. Make sure it is exported in your ~/.bashrc"
fi

# 4. Create a Python Virtual Environment for readrate.sh dependencies
echo "Setting up Python virtual environment for readrate.sh..."
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
fi

# Install dependencies inside the virtual environment
./.venv/bin/pip install --upgrade pip --quiet
./.venv/bin/pip install opcua-client asyncua --quiet

echo "=== Setup Complete. All dependencies and build hooks are in place. ==="