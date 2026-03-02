#!/usr/bin/env bash
set -e

# Safe local demo: starts a local banner server and probes it.
python3 scripts/local_banner_server.py --port 9090 --banner "HELLO-FROM-LOCAL\r\n" &
PID=$!
sleep 0.2

./bannerhunter --i-have-permission -h 127.0.0.1 -p 9090 --no-tls --timeout 2

kill $PID >/dev/null 2>&1 || true
