# Architecture

This project is small on purpose, but structured for maintainability.

## Modules

- `src/main.c`
  - CLI parsing (`getopt_long`)
  - Applies safety constraints (permission flag, max-ports cap)
  - Dispatches scans and formats output (text / JSONL)

- `src/ports.c`
  - Parses port specs like `80,443,8000-8100`
  - De-duplicates ports and enforces bounds

- `src/net.c`
  - Resolves IPv4/IPv6 (`getaddrinfo`)
  - Non-blocking connect with timeout (`select`)
  - Applies socket timeouts for reads/writes

- `src/tls.c`
  - OpenSSL wrapper for client handshakes
  - SNI support
  - Optional peer certificate subject/issuer extraction

- `src/probe.c`
  - Plain/TLS write + read helpers
  - Banner read
  - Optional HTTP HEAD request for service identification
  - Minimal sanitization (avoid control chars breaking terminals/logs)

- `src/scan.c`
  - Orchestrates per-port behavior:
    - connect
    - TLS attempt (if enabled)
    - fallback to plain (AUTO mode)
    - optional pacing delay

## Safety design decisions

- Requires `--i-have-permission`
- Single host only (no IP ranges)
- Small default port set
- Hard maximum of 256 ports even if user requests more
- Optional pacing delay so it can be used gently on production systems

## Extensibility ideas (safe)

- Add `--resolve-only` to print resolved addresses without connecting
- Add `--ports-file` to load a small curated port list from disk
- Add `--output-dir` for per-port logs (still single-target)
- Add “fingerprints” as **local heuristics** (no active exploitation)

If you add features, keep the “single-target, permission-gated” ethos.
