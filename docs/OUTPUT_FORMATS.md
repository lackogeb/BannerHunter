# Output Formats

BannerHunter supports **text** and **JSON Lines** output.

## Text output (default)

Example (format may vary slightly):
```
example.com:443  OPEN  TLS  HTTP/1.1 200 OK ...
  cert_subject: /CN=example.com/...
  cert_issuer : /C=US/O=...
example.com:22   OPEN  PLAIN  SSH-2.0-OpenSSH_9.6
example.com:25   CLOSED (connect timeout)
```

## JSONL output (`--jsonl`)

Each port emits one JSON object on its own line:
```json
{"host":"example.com","port":443,"tcp_ok":true,"used_tls":true,"tls_ok":true,"banner":"HTTP/1.1 200 OK","cert_subject":"/CN=example.com","cert_issuer":"/C=US/...","error":""}
```

Why JSONL?
- Works well with `jq`, `grep`, `awk`
- Easy to append to log files
- Stream-friendly

## Appending to a file (`--output`)

Text:
```bash
bannerhunter ... --output results.txt
```

JSONL:
```bash
bannerhunter ... --jsonl --output results.jsonl
```
