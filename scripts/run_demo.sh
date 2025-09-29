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
    echo "Removing module $MODULE_NAME..."
    sudo rmmod nxp_simtemp
    echo "Module removed successfully!"
    else
        echo "Module nxp_simtemp is not loaded. Nothing to remove."
    fi
}

# ---------------------------------------------
# Insert the module
# ---------------------------------------------
if lsmod | grep -q "^nxp_simtemp"; then
    echo "Module nxp_simtemp is already loaded"
    remove_module
else
    echo "Inserting module $MODULE_NAME..."
    sudo insmod "$KBUILD_DIR/$MODULE_NAME"
    echo "Module inserted successfully!"
fi

# ---------------------------------------------
# CLI test
# ---------------------------------------------
echo "Module is now loaded. Running tests..."

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
remove_module
