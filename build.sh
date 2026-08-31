#!/bin/bash
set -e

# Configuration
PORT="/dev/ttyACM0"
CMAKE_FILE="CMakeLists.txt"
PROJECT_NAME=$(basename "$PWD")
BACKUP_FILE="../${PROJECT_NAME}_backup.tar.gz"

echo "=== Checking for changes to backup ==="
touch -m -d "20 minutes ago" /tmp/reference_marker

if [ ! -f "$BACKUP_FILE" ]; then
    echo "No backup found. Creating fresh backup at $BACKUP_FILE..."
    tar --exclude='./build' -czf "$BACKUP_FILE" .
elif [ "/tmp/reference_marker" -nt "$BACKUP_FILE" ]; then
    if [ -n "$(find . -path ./build -prune -o -newer "$BACKUP_FILE" -print -quit)" ]; then
        echo "Changes found. Freshening backup..."
        
        # tar cannot update a compressed archive, so we unzip, update, and re-zip
        gunzip "$BACKUP_FILE"
        TAR_FILE="../${PROJECT_NAME}_backup.tar"
        
        # -u appends files that are newer than their archive copy
        tar --exclude='./build' -uf "$TAR_FILE" .
        gzip "$TAR_FILE"
    else
        echo "No changes since last backup."
    fi
else
        echo "Backup is less than 20 minutes old."
fi
if [ -d "build" ]; then
    # Check if the CMake file is newer than the generated CMakeCache
    if [ -f "$CMAKE_FILE" ] && [ "$CMAKE_FILE" -nt "build/CMakeCache.txt" ]; then
        echo "$CMAKE_FILE has changed since last build. Cleaning build directory..."
        rm -rf build
    fi
fi

if [ ! -d "build" ]; then
    echo "=== Creating build directory ==="
    mkdir build
fi

cd build

echo "=== Building ==="
cmake .. 
make -j4

# If the RP2350 is not mounted as a mass storage device, wait for the serial port
if [ ! -d "/media/shawn/RP2350" ]; then
    echo "=== Waiting for device ==="
    echo "Waiting for $PORT"
    while [ ! -e "$PORT" ]; do
        echo -n "."
        sleep 1
    done
    echo ""
fi

echo "=== Flashing Pico ==="
# Prefer .uf2, fallback to .elf
UF2_FILE=$(ls -t *.uf2 2>/dev/null | head -n1)
ELF_FILE=$(ls -t *.elf 2>/dev/null | head -n1)

if [ -n "$UF2_FILE" ]; then
    echo "Flashing: $UF2_FILE"
    sudo picotool load "$UF2_FILE" -fx
elif [ -n "$ELF_FILE" ]; then
    echo "Flashing: $ELF_FILE"
    sudo picotool load "$ELF_FILE" -fx
else
    echo "Error: No .uf2 or .elf file found in build/"
    exit 1
fi

# Check if minicom is already running
if pgrep -x "minicom" > /dev/null; then
    echo "minicom is running in the background. Skipping 'cat $PORT'."
else
    echo "Connecting to $PORT... Press Ctrl+C to stop."
    sleep 1
    cat "$PORT"
fi
