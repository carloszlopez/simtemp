#!/bin/bash
set -e
set -o pipefail

MODULE_NAME="nxp_simtemp"
SCRIPT_DIR="$(pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."

print_header() {
    local line="$1"
    echo
    echo "=============================="
    echo "$line"
    echo "=============================="
    echo
}

# =============================================
# Log
# =============================================
LOG_FILE="$SCRIPT_DIR/$MODULE_NAME.log"
> $LOG_FILE
exec > >(tee -a "$LOG_FILE") 2>&1

print_header "Log file created in $LOG_FILE"

# =============================================
# KERNEL
# =============================================
SOURCE_DIR="$PROJECT_ROOT/kernel"
OUT_NAME="$MODULE_NAME.ko"

print_header "$OUT_NAME build... "

# Check for Makefile
if [ ! -f "$SOURCE_DIR/Makefile" ]; then
    echo "No Makefile found in $SOURCE_DIR"
    exit 1
fi

if [ "$1" == "clean" ]; then
    # Clean
    echo "Cleaning..."
    make -C "$SOURCE_DIR" clean
else
    # Build
    echo "Building..."
    make -C "$SOURCE_DIR"
fi

print_header "$OUT_NAME built successfully!"

# =============================================
# CLI
# =============================================

SOURCE_DIR="$PROJECT_ROOT/user/cli"
OUT_NAME="${MODULE_NAME}_cli"

print_header "$OUT_NAME build.... "

# Check for Makefile
if [ ! -f "$SOURCE_DIR/Makefile" ]; then
    echo "No Makefile found in $SOURCE_DIR"
    exit 1
fi


if [ "$1" == "clean" ]; then
    echo "Clean build..."
    make -C "$SOURCE_DIR" clean
else
    echo "Building C++ CLI..."
    make -C "$SOURCE_DIR"
fi

print_header "$OUT_NAME built successfully!"
