# Changelog

## Unreleased (expanded project)

### Added
- Modular C implementation (`src/`, `include/`) with clearer separation of concerns
- IPv4/IPv6 resolution and non-blocking connect with timeout
- Port spec parsing (`80,443,8000-8100`) with de-dup + safety caps
- Output formats: text + JSON Lines (`--jsonl`)
- Optional output file append (`--output`)
- Optional HTTP HEAD probe (`--probe-http`)
- Optional banner trigger newline (`--send-newline`)
- TLS SNI override (`--sni`) and optional certificate subject/issuer (`--show-cert`)
- Docs folder (usage, architecture, output formats, FAQ, Termux)
- Local demo scripts and local tests
- GitHub Actions CI workflow
- Man page (`man/bannerhunter.1`)
- Legacy single-file code archived under `legacy/`

### Changed
- Reworked installer scripts to build and install cleanly
- Updated README with capability overview and safety posture

### Security
- Permission gating enforced via `--i-have-permission`
- Hard max port count and default pacing delay

