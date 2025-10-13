#!/usr/bin/env bash
set -euo pipefail

mkfifo /tmp/fifo_temp 2>/dev/null || true
mkfifo /tmp/fifo_humedad 2>/dev/null || true
mkfifo /tmp/fifo_viento 2>/dev/null || true
mkfifo /tmp/fifo_lluvia 2>/dev/null || true

rm -f /tmp/sensor_*.pid

./sensor_temp_sig &
./sensor_humedad_sig &
./sensor_viento_sig &
./sensor_lluvia_sig &

sleep 1

./central_signal
