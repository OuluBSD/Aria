# Aria & WebDriver

A high-performance autonomous web navigation, scraping, and browser automation suite natively built in C++ using the [Ultimate++ (U++)](https://www.ultimatepp.org) framework.

## Packages

### 1. `WebDriver`
A complete, native C++ W3C WebDriver client with zero external HTTP or JSON dependencies beyond standard U++ Core and Net libraries.
- **W3C WebDriver Specification**: Full coverage of session lifecycle, window management, element locating (`By::Id`, `By::Css`, `By::XPath`), cookies, screenshots, and JavaScript evaluation.
- **Cross-Browser Support**: First-class support for Firefox (`geckodriver`) and Chromium/Chrome (`chromedriver`).
- **Undetected & Stealth Automation**:
  - Native CDP (Chrome DevTools Protocol) stealth injection.
  - Marionette script hooks for anti-fingerprint detection bypass.
  - Automated user-agent, viewport, and navigator property emulation (`StealthJS`).

### 2. `Aria`
High-level scraping, extraction, and interaction orchestration layer built on top of `WebDriver`.
- **Platform Scrapers**:
  - **Discord**: Channel message history, thread scraping, attachment harvesting (`DiscordScraper`).
  - **GitHub**: Issues, pull requests, repository releases, discussion extraction (`GitHubScraper`).
  - **Slack**: Workspace channels, thread extraction, user directory sync (`SlackScraper`).
  - **SourceForge**: Project summary, issue tracker, mailing list archives (`SourceForgeScraper`).
  - **Manager Skylark**: Local project tracker web UI scraper and test runner (`ManagerSkylarkScraper`).
  - **Social & Media**: Threads, YouTube, Suno, Forex economic calendar, Duunitori/Työmarkkinatori.
- **Navigator**: Resilient high-level navigation helper with automatic retries, wait conditions, and human-like typing/scrolling delays.
- **Security & Safety**: Masked credential storage (`CredentialManager`), domain allowlists, and execution bounds.

## Layout

```
Aria/
├── Aria/                  # High-level scraping & automation platform
│   ├── Aria.h            # Main header
│   ├── Aria.upp          # Package manifest
│   ├── Navigator.{h,cpp} # Human-emulated navigation controller
│   ├── DiscordScraper.{h,cpp}
│   ├── GitHubScraper.{h,cpp}
│   ├── SlackScraper.{h,cpp}
│   └── SourceForgeScraper.{h,cpp}
├── WebDriver/             # Native W3C WebDriver client
│   ├── WebDriver.h       # Master client header
│   ├── WebDriver.upp     # Package manifest
│   ├── Session.{h,cpp}   # Browser session management
│   ├── Element.{h,cpp}   # DOM element queries and interactions
│   ├── Undetected.{h,cpp}# Stealth and bot-detection bypass
│   └── Firefox.{h,cpp}   # Gecko / Marionette driver specialization
├── docs/                  # Architectural guidelines
├── LICENSE                # License terms
└── README.md
```

## Prerequisites

Ensure a compatible WebDriver binary is running:
- **Firefox / Marionette**:
  ```bash
  geckodriver --port 4444
  ```
- **Chrome / Chromium**:
  ```bash
  chromedriver --port 9515
  ```

## Building

Build with `umk` or `uppbuilder`:
```bash
# Build native WebDriver client
umk Aria WebDriver <BUILD_METHOD> -brv

# Build Aria scraper suite
umk Aria Aria <BUILD_METHOD> -brv
```
