# Termux Notes

## Install build requirements
```bash
pkg update
pkg install clang make openssl
```

## Build
```bash
make
```

## Install
```bash
chmod +x install.sh
./install.sh
```

## Common issues

### “openssl/ssl.h not found”
Install OpenSSL headers:
```bash
pkg install openssl
```

### Permission denied running scripts
```bash
chmod +x install.sh uninstall.sh
```
