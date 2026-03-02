#!/data/data/com.termux/files/usr/bin/bash
set -e

PREFIX_DIR="${PREFIX:-/data/data/com.termux/files/usr}"
BIN_DIR="${PREFIX_DIR}/bin"

echo "[*] Removing ${BIN_DIR}/bannerhunter"
rm -f "${BIN_DIR}/bannerhunter" || true

echo "[+] Uninstalled."
