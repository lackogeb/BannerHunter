#!/usr/bin/env python3
"""
Local demo server for BannerHunter tests.

Starts a TCP server on 127.0.0.1:<port> that immediately sends a banner and closes.

This is for *local* testing only.
"""
import argparse, socket, threading

def handle(conn, banner: bytes):
    try:
        conn.sendall(banner)
    finally:
        conn.close()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=9090)
    ap.add_argument("--banner", default="DEMO-BANNER/1.0\r\n")
    args = ap.parse_args()

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", args.port))
    s.listen(50)
    print(f"[demo] listening on 127.0.0.1:{args.port}")
    banner = args.banner.encode("utf-8", "replace")

    try:
        while True:
            conn, _ = s.accept()
            threading.Thread(target=handle, args=(conn,banner), daemon=True).start()
    except KeyboardInterrupt:
        pass
    finally:
        s.close()

if __name__ == "__main__":
    main()
