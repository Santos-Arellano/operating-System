#!/usr/bin/env bash
set -euo pipefail

mkfifo /tmp/fifo_temp 2>/dev/null || true
mkfifo /tmp/fifo_humedad 2>/dev/null || true
mkfifo /tmp/fifo_viento 2>/dev/null || true
mkfifo /tmp/FIFO_LLUVIA 2>/dev/null || true

./central &
CENTRAL_PID=$!
echo "[run] central PID=$CENTRAL_PID"
sleep 1

./sensor_temp &
./sensor_humedad &
./sensor_viento &
./sensor_lluvia &

echo "[run] Sensores lanzados. Ctrl+C para parar (y luego pkill -f sensor_)"
wait $CENTRAL_PID
