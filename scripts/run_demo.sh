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
    if lsmod | grep -q "^nxp_simtemp"; then
    sudo rmmod nxp_simtemp
    fi
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
echo "Running tests... (Press Ctrl+C to stop)"
# Run CLI in background
sudo "$CLI_EXE" &
CLI_PID=$!

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
wait $CLI_PID
echo "Removing $MODULE_NAME..."
remove_module
echo "Module removed successfully!"
