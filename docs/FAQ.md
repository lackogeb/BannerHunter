# FAQ

## Why do I have to pass `--i-have-permission`?
Because banner probing can be misused. This flag is a deliberate friction point that keeps the project oriented around legitimate, authorized testing.

## Why does TLS sometimes fail but plain TCP works?
Some services do not speak TLS on that port, or they require a specific SNI value. Try:
- `--sni your.hostname`
- or `--no-tls` if you know it is plain

## Why do some ports show “OPEN” but no banner?
Many services don’t send banners unless you speak the protocol first, or they only respond after authentication. BannerHunter avoids protocol-heavy interaction to remain safe and minimal.

Try `--probe-http` for web services, or `--send-newline` for simple line-based services.

## Is this a vulnerability scanner?
No. It is a connection + identification helper.

## Can I scan multiple hosts?
Not in this project’s default scope. Keeping it single-target helps reduce misuse and accidental harm.
