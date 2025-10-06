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

run_cli_test() {
    local cmd="$1"
    local description="$2"

    echo -e "\n$description..."
    set +e
    sudo "$CLI_EXE" $cmd
    local RET=$?
    set -e

    if [ $RET -eq 0 ]; then
        echo "$description successful!"
    else
        echo "$description failed!"
        remove_module
        exit 1
    fi
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
run_cli_test "readattr sampling_ms" "Reading sampling_ms"
run_cli_test "readattr threshold_mC" "Reading threshold_mC"
run_cli_test "readattr mode" "Reading mode"
run_cli_test "testmode" "Test mode"

echo " "
echo "Reading temperature... "
sudo "$CLI_EXE" read &
CLI_PID=$!

sleep 5
run_cli_test "writeattr mode 1" "Writing NOISY mode"
sleep 5
run_cli_test "writeattr mode 2" "Writing RAMP mode"
sleep 5

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
wait $CLI_PID
remove_module
