# AGENTS - Aria & WebDriver

Guide for AI agents working in this repository.

## Overview
- **Repository**: `OuluBSD/Aria`
- **Packages**: `Aria/`, `WebDriver/`
- **Framework**: Ultimate++ (U++)

## Architecture & Responsibilities
- `WebDriver/`: Low-level W3C WebDriver wire protocol implementation.
  - Communicates over HTTP with `geckodriver` (default `:4444`) or `chromedriver` (default `:9515`).
  - Implements stealth scripts (`StealthJS.h`, `Undetected.cpp`) to defeat browser fingerprinting.
  - Exposes `Session`, `Element`, `Finder`, `By`, and `Capabilities`.
- `Aria/`: High-level automation and scraping suite.
  - Provides site-specific scrapers (Discord, GitHub, Slack, Skylark, SourceForge, etc.).
  - Orchestrates human-user emulation (keystroke pacing, smooth scrolling, random pauses).
  - Handles credential management and logging.

## Conventions
- Preserve commit history and attribution.
- Ensure package `.upp` manifests list `AGENTS.md` as the first entry in `file`.
- Avoid hardcoding sensitive API keys or passwords; use environment variables or `CredentialManager`.
- Plan files belong to the user; do not commit them to this repository.
