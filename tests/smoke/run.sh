#!/usr/bin/env bash
# Headless smoke test: start LCDHost with the default layout and the core
# plugins enabled, then check that it neither crashes nor fails to load them.
# Usage: tests/smoke/run.sh <path to LCDHost.app/bin> [seconds]
set -u

BIN_DIR=$(cd "$1" && pwd)
DURATION=${2:-20}
SRC_DIR=$(cd "$(dirname "$0")/../.." && pwd)
PLUGINS="Bar Decor Dial Graph Image Text VirtualLCD"

export HOME=$(mktemp -d)
export QT_QPA_PLATFORM=offscreen
mkdir -p "$HOME/Documents/LCDHost" "$HOME/.config/Link Data"
cp -r "$SRC_DIR/layouts" "$HOME/Documents/LCDHost/"
{
    echo '[plugins]'
    for p in $PLUGINS; do echo "libLH_$p.so.1.0\\enabled=true"; done
} > "$HOME/.config/Link Data/LCDHost.conf"

LOG="$HOME/lcdhost.log"
# Run from an unrelated directory so library lookup cannot rely on the cwd.
(cd / && timeout "$DURATION" "$BIN_DIR/LCDHost" > "$LOG" 2>&1)
status=$?

failed=0
if [ "$status" -ne 124 ]; then
    echo "FAIL: LCDHost exited early with status $status"
    failed=1
fi
for p in $PLUGINS; do
    if ! grep -q "\"LH_$p\" loaded with" "$LOG"; then
        echo "FAIL: plugin LH_$p did not load"
        failed=1
    fi
done
if [ "$failed" -ne 0 ]; then
    echo "--- log tail ---"
    tail -n 50 "$LOG"
    exit 1
fi
echo "OK: LCDHost ran ${DURATION}s and loaded: $PLUGINS"
