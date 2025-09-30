# =============================================
# Build kernel module
# =============================================
set -e
set -o pipefail

MODULE_NAME="nxp_simtemp"
SCRIPT_DIR="$(pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."
KERNEL_DIR="$PROJECT_ROOT/kernel"
KBUILD_DIR="$KERNEL_DIR/kbuild"
LOG_FILE="$KBUILD_DIR/$MODULE_NAME.log"
KERNEL_HEADERS_DIR="/lib/modules/$(uname -r)/build"

# Log file
if [ ! -d "$KBUILD_DIR" ]; then
    mkdir $KBUILD_DIR
fi
> "$LOG_FILE"
exec > >(tee -a "$LOG_FILE") 2>&1

# Header
echo "                              "
echo "=============================="
echo "Starting build for $MODULE_NAME"
echo "=============================="
echo "                              "

# Build (clean if needed)
if [ ! -f "$KERNEL_DIR/Makefile" ]; then
    echo "No Makefile found in $KERNEL_DIR"
else        
    if [ "$1" == "clean" ]; then
        echo "Clean build..."
        find "$KBUILD_DIR" -mindepth 1 ! -name "*.log" -exec rm -rf {} +
    fi
    echo "Building..."
    rsync -au --include='*.c' --include='*.h' --include='Makefile' \
        --exclude='*' "$KERNEL_DIR/" "$KBUILD_DIR/"
    make -C "$KERNEL_HEADERS_DIR" M="$KBUILD_DIR" modules
fi

# Footer
echo "                              "
echo "=============================="
echo "Build finished successfully!"
echo "Kernel module: $KBUILD_DIR/$MODULE_NAME.ko"
echo "Build log: $LOG_FILE"
echo "=============================="
echo "                              "