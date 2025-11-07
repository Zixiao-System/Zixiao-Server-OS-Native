#!/bin/bash
# Run Zixiao OS x86_64 kernel in QEMU

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Build kernel if needed
if [ ! -f build/zixiao-x86_64.elf ]; then
    echo "Building x86_64 kernel..."
    make x86_64
fi

echo "Starting Zixiao OS (x86_64) in QEMU..."
echo "Press Ctrl+C to exit"
echo ""

# Try to run with ISO first, fall back to direct kernel boot
if [ -f build/zixiao-x86_64.iso ]; then
    qemu-system-x86_64 \
        -cdrom build/zixiao-x86_64.iso \
        -m 512M \
        -serial stdio \
        -no-reboot \
        -no-shutdown
else
    echo "Warning: ISO not found, using direct kernel boot"
    qemu-system-x86_64 \
        -kernel build/zixiao-x86_64.elf \
        -m 512M \
        -serial stdio \
        -no-reboot \
        -no-shutdown
fi
