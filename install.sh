#!/data/data/com.termux/files/usr/bin/bash
set -e

# BannerHunter installer (Termux + Linux)
# Use only on systems you own or have explicit permission to test.

PREFIX_DIR="${PREFIX:-/data/data/com.termux/files/usr}"
BIN_DIR="${PREFIX_DIR}/bin"

echo "[*] Building BannerHunter..."
make clean >/dev/null 2>&1 || true
make

echo "[*] Installing to: ${BIN_DIR}"
mkdir -p "${BIN_DIR}"
cp -f bannerhunter "${BIN_DIR}/bannerhunter"
chmod +x "${BIN_DIR}/bannerhunter"

echo "[+] Installed: ${BIN_DIR}/bannerhunter"
echo "[i] Run: bannerhunter --help"
