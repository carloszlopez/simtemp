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

remove_module() {
    if lsmod | grep -q "^nxp_simtemp"; then
    sudo rmmod nxp_simtemp
    fi
}

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
echo "Running tests..."
sudo cat /dev/nxp_simtemp

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
echo "Removing $MODULE_NAME..."
remove_module
echo "Module removed successfully!"
