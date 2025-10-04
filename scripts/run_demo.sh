#!/bin/bash
# =============================================
# test_module.sh - `insmod` → configure → run CLI test → `rmmod`; return non‑zero on failure.
# =============================================

set -e
set -o pipefail

# Determine paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."
KBUILD_DIR="$PROJECT_ROOT/kernel/kbuild"
MODULE_NAME="nxp_simtemp.ko"
CLI_EXE="$PROJECT_ROOT/user/cli/nxp_simtemp_cli"

remove_module() {
    echo "Removing $MODULE_NAME..."
    if lsmod | grep -q "^nxp_simtemp"; then
    sudo rmmod nxp_simtemp
    fi
    echo "Module removed successfully!"
}

cleanup() {
    echo "Interrupted. Cleaning up..."
    if [[ -n "$CLI_PID" ]]; then
        kill "$CLI_PID" 2>/dev/null || true
        wait "$CLI_PID" 2>/dev/null || true
    fi
    remove_module
    exit 1
}

trap cleanup SIGINT SIGTERM

# ---------------------------------------------
# Insert the module
# ---------------------------------------------
echo "Instering $MODULE_NAME..."
if lsmod | grep -q "^nxp_simtemp"; then
    remove_module
else
    sudo insmod "$KBUILD_DIR/$MODULE_NAME"
    echo "Module inserted successfully!"
fi

# ---------------------------------------------
# CLI test
# ---------------------------------------------
echo "Reading sampling_ms... "
set +e
sudo "$CLI_EXE" readattr sampling_ms
RET=$?
set -e
if [ $RET -eq 0 ]; then
    echo "sampling_ms read successfully!"
else
    echo "Failed to read sampling_ms"
    remove_module
    exit 1
fi

echo "Reading threshold_mC... "
set +e
sudo "$CLI_EXE" readattr threshold_mC
RET=$?
set -e
if [ $RET -eq 0 ]; then
    echo "threshold_mC read successfully!"
else
    echo "Failed to read threshold_mC"
    remove_module
    exit 1
fi

echo "Reading temperature... "
sudo "$CLI_EXE" read &
CLI_PID=$!

echo "TEST ... "
sleep 5

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
wait $CLI_PID
remove_module
