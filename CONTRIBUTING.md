# Contributing

Thanks for considering a contribution.

## Ground rules
- Only propose changes that support **legitimate, authorized** use.
- Avoid features that materially increase misuse potential (e.g., mass scanning or stealth features).
- Keep changes small and well-documented.

## Development
### Build
```bash
make
```

### Run
```bash
./bannerhunter --i-have-permission example.com 80 443
```

### Style
- Prefer clear error handling over cleverness
- Keep output stable and parseable
- Avoid undefined behavior and check return values

## Pull Requests
- Explain *why* the change matters
- Include example output or usage
- Keep dependencies minimal
