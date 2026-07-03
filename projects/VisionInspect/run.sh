#!/bin/bash
export QT_QPA_PLATFORM=xcb
export DISPLAY=$DISPLAY
if [ -d /mnt/wslg/runtime-dir ]; then
    chmod 0700 /mnt/wslg/runtime-dir 2>/dev/null || true
fi
exec ./build/VisionInspect "$@"
