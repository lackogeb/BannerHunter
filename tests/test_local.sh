#!/usr/bin/env bash
set -e

echo "[*] Building..."
make clean >/dev/null 2>&1 || true
make

echo "[*] Running local demo test..."
bash scripts/run_local_demo.sh

echo "[+] OK"
