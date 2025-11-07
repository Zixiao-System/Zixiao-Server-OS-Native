#!/bin/bash
# Run Zixiao OS ARM64 kernel in QEMU

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Build kernel if needed
if [ ! -f build/zixiao-arm64.elf ]; then
    echo "Building ARM64 kernel..."
    make arm64
fi

echo "Starting Zixiao OS (ARM64) in QEMU..."
echo "Press Ctrl+A then X to exit"
echo ""

qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a57 \
    -kernel build/zixiao-arm64.elf \
    -m 512M \
    -nographic \
    -serial mon:stdio
