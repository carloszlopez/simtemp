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
SOURCE_DIR="$PROJECT_ROOT/kernel"
KERNEL_OUT_NAME="$MODULE_NAME.ko"

remove_module() {
    echo "Removing $KERNEL_OUT_NAME module..."
    if lsmod | grep -q "^$MODULE_NAME"; then
    sudo rmmod $KERNEL_OUT_NAME
    fi
    echo "Removed successfully!"
}

print_header "Instering $KERNEL_OUT_NAME..."
echo "Try removing $KERNEL_OUT_NAME in case is already loaded... "
remove_module
sudo insmod "$SOURCE_DIR/$KERNEL_OUT_NAME"
print_header "$KERNEL_OUT_NAME inserted successfully!"

# ---------------------------------------------
# CLI
# ---------------------------------------------
SOURCE_DIR="$PROJECT_ROOT/user/cli"
CLI_OUT_NAME="${MODULE_NAME}_cli"
READ_TIME=5

run_cli_test() {
    local cmd="$1"
    local description="$2"

    echo -e "\n$description..."
    set +e
    sudo "$SOURCE_DIR/$CLI_OUT_NAME" $cmd
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

print_header "$CLI_OUT_NAME test mode..."
run_cli_test "testmode" "Test mode"
print_header "$CLI_OUT_NAME test mode successfully!"

print_header "$CLI_OUT_NAME read /dev for $READ_TIME seconds..."
sudo "$SOURCE_DIR/$CLI_OUT_NAME" read &
CLI_PID=$!
sleep $READ_TIME
sudo kill $CLI_PID
print_header "$CLI_OUT_NAME read /dev finished successfully!"

# ---------------------------------------------
# GUI
# ---------------------------------------------
BIN_DIR="$PROJECT_ROOT/user/gui/build/Desktop_Qt_6_9_3-Debug"
GUI_OUT_NAME="gui"

print_header "$BIN_DIR/$GUI_OUT_NAME opening..."
set +e
sudo "$BIN_DIR/$GUI_OUT_NAME" &
GUI_PID=$!
wait $GUI_PID
set -e
print_header "$BIN_DIR/$GUI_OUT_NAME finished successfully!"

# ---------------------------------------------
# Remove the module
# ---------------------------------------------
remove_module
