# BannerHunter

**BannerHunter** is a **safety-first, single-target** TCP banner & service probe. It connects to a **host you are authorized to test**, optionally negotiates **TLS**, and attempts to read a **banner** (plus an optional **HTTP HEAD probe** to identify web services).

> ⚠️ **Authorized use only.**  
> This project is designed for legitimate troubleshooting, asset inventory, and approved security testing.  
> Do **not** scan systems you don’t own or don’t have explicit permission to test.

---

## What it can do (Capabilities)

### Core capabilities
- **Single-target probing** (one host at a time) for safer, more deliberate testing
- **IPv4 + IPv6** support via `getaddrinfo()`
- **Connect timeout** using non-blocking connect + `select()`
- **Read banner** (when services present one)
- **Optional CRLF send** (`--send-newline`) to trigger banners on some services
- **Optional HTTP HEAD probe** (`--probe-http`) to identify web servers even if they don’t send a banner
- **TLS support** (OpenSSL):
  - `AUTO`: try TLS first, then fall back to plain TCP
  - `--tls`: TLS only
  - `--no-tls`: plain TCP only
  - **SNI override** with `--sni`
  - Optional **certificate subject/issuer** output (`--show-cert`)
- **Output modes**
  - Human-friendly text (default)
  - **JSON Lines** (`--jsonl`) for logging and pipelines
  - Optional file append (`--output <file>`)
- **Safety controls**
  - Requires explicit **authorization acknowledgement**: `--i-have-permission`
  - Built-in **hard caps** (defaults + `--max-ports`, max 256)
  - Optional **delay** between ports (`--delay-ms`) to reduce load

### What it intentionally does *not* do
- No multi-host scanning, CIDR sweeping, or “spray” modes
- No exploitation, brute force, credential guessing, or vulnerability payloads

---

## Project layout

```
.
├── include/                 # public headers
├── src/                     # implementation
├── docs/                    # detailed documentation
├── examples/                # sample outputs and usage
├── scripts/                 # local demos (safe)
├── tests/                   # local-only tests (safe)
├── legacy/                  # historical single-file implementation
├── man/                     # man page source
├── Makefile
├── install.sh
└── uninstall.sh
```

---

## Build

### Linux / macOS (with OpenSSL dev headers)
```bash
make
```

### Termux
```bash
pkg update
pkg install clang make openssl
make
```

---

## Install / Uninstall

### Termux
```bash
chmod +x install.sh uninstall.sh
./install.sh
```

Uninstall:
```bash
./uninstall.sh
```

---

## Usage

**You must pass `--i-have-permission` or the tool will exit.**

Examples:

Scan curated defaults with HTTP probe:
```bash
bannerhunter --i-have-permission -h example.com --default-ports --probe-http
```

Scan specific ports, TLS only, print certificate subject/issuer:
```bash
bannerhunter --i-have-permission -h example.com -P 443,8443 --tls --show-cert
```

Plain TCP only, slower/safer pacing:
```bash
bannerhunter --i-have-permission -h 10.0.0.5 -P 22,80,8080 --no-tls --delay-ms 250
```

Full CLI help:
```bash
bannerhunter --help
```

---

## Documentation

Start here:
- `docs/USAGE.md`
- `docs/ARCHITECTURE.md`
- `docs/OUTPUT_FORMATS.md`
- `docs/FAQ.md`
- `docs/TERMUX.md`

---

## Contributing

See:
- `CONTRIBUTING.md`
- `CODE_OF_CONDUCT.md`

---

## License

See `LICENSE.md`.
