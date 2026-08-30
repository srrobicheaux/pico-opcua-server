#!/bin/bash
set -e

echo "=== Pico Universal Access: Full Environment Setup ==="

# Initialize variables for Wi-Fi credentials
WIFI_SSID=""
WIFI_PASSWORD=""

# Parse command-line arguments for credentials and help
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -s|--ssid) WIFI_SSID="$2"; shift 2 ;;
        -p|--password) WIFI_PASSWORD="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: ./setup.sh [options]"
            echo "Options:"
            echo "  -s, --ssid <ssid>       Wi-Fi Network SSID"
            echo "  -p, --password <pwd>    Wi-Fi Network Password"
            echo "  -h, --help              Display this help message"
            exit 0
            ;;
        *) echo "Unknown parameter: $1"; exit 1 ;;
    esac
done

# Handle secrets.h logic based on file presence and parameters
if [ -f "secrets.h" ]; then
    echo "secrets.h already exists. Using existing configuration."
else
    if [ -n "$WIFI_SSID" ] && [ -n "$WIFI_PASSWORD" ]; then
        echo "Generating secrets.h from command-line arguments..."
        echo "#define WIFI_SSID       \"$WIFI_SSID\"" > secrets.h
        echo "#define WIFI_PASSWORD   \"$WIFI_PASSWORD\"" >> secrets.h
    else
        echo "Error: secrets.h is missing and Wi-Fi credentials were not provided."
        echo "Usage: ./setup.sh -s <SSID> -p <PASSWORD>"
        exit 1
    fi
fi

# 1. Clone FreeRTOS Kernel 
if [ -d "FreeRTOS-Kernel" ]; then
  rm -rf FreeRTOS-Kernel
fi
    echo "Cloning FreeRTOS Kernel..."
    git clone --recursive https://github.com/FreeRTOS/FreeRTOS-Kernel.git --depth 1

# 2. Clone open62541 
if [ -d "open62541" ]; then
  rm -rf open62541
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

echo "=== Setup Complete. All dependencies and build hooks are in place. ==="

# Optional prompt to run build.sh upon successful completion
read -p "Setup complete. Would you like to run build.sh now? (y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -f "./build.sh" ]; then
        ./build.sh
    else
        echo "Error: build.sh not found in the current directory."
    fi
fi