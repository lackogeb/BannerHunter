# Usage Guide

BannerHunter is intentionally **single-target** and **permission-gated**.

## Quick start

```bash
bannerhunter --i-have-permission -h example.com --default-ports --probe-http
```

## Options in plain language

### Required
- `--i-have-permission`  
  You confirm you own the target or have written authorization to test it.

- `-h, --host <host>`  
  Hostname or IP. IPv4 and IPv6 are supported.

### Port selection
- `--default-ports`  
  Uses a curated set of common service ports (small enough to be safe by default).

- `-p, --port <n>`  
  Add a single port (repeatable).

- `-P, --ports <spec>`  
  Add comma-separated ports and/or ranges:
  - `80,443,8080`
  - `20-25`
  - `22,80,443,8000-8100`

### TLS behavior
- Default mode is **AUTO**:
  - Try TLS handshake first
  - If it fails, fall back to plain TCP
- `--tls` forces TLS only
- `--no-tls` forces plain TCP only
- `--sni <name>` sets TLS SNI (useful for virtual hosts / CDNs)
- `--show-cert` prints peer cert subject/issuer (when provided)

### Probing & banner tricks
- `--send-newline` sends `\r\n` after connect (some services only show banners after input)
- `--probe-http` sends an HTTP `HEAD /` request and reads a response
  - Helps identify HTTP services on non-standard ports

### Safety & performance
- `-t, --timeout <sec>` sets per-port timeouts (connect/read/write)
- `--delay-ms <ms>` delays between ports
- `--max-ports <n>` hard cap; max is 256

### Output
- Default is text
- `--jsonl` emits JSON Lines (one record per port)
- `-o, --output <file>` appends to a file (works with both text and JSONL)
- `-q, --quiet` reduces non-result chatter

## Typical workflows

### “What’s running on this host?”
```bash
bannerhunter --i-have-permission -h myserver --default-ports --probe-http
```

### “Check only web ports”
```bash
bannerhunter --i-have-permission -h myserver -P 80,443,8080,8443 --probe-http
```

### “My service is TLS but the hostname differs from the IP”
```bash
bannerhunter --i-have-permission -h 203.0.113.10 -p 443 --tls --sni example.com --show-cert
```
