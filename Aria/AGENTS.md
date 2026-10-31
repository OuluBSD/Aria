# Aria Package AGENTS Guide

## Scope
- Applies to `uppsrc/Aria` and its entire sub-tree

## Purpose
- AI-driven orchestration layer for web interaction and automation
- Enables users to delegate complex web tasks to AI agents
- Combines web browsing, data extraction, and decision-making into a unified experience
- Part of the Ultimate++ ecosystem for efficient digital productivity

## Key Areas
- **Orchestration Engine**:
  - `Aria` - Main orchestrator class coordinating all subsystems
  - `AriaNavigator` - Handles web navigation and page interaction
  - `ScriptManager` - Manages user-defined scripts and workflows

- **Management Services**:
  - `ScriptManager` - Script execution and scheduling
  - `SafetyManager` - Compliance and risk management
  - `ReportManager` - Generates reports from executed tasks
  - `SiteManager` - Manages monitored websites and domains
  - `PluginManager` - Extensible plugin system for custom extensions

- **Integration Layer**:
  - Multiple web drivers (Selenium-based) for browser automation
  - Scrapers for popular platforms (Google, WhatsApp, Discord, Twitter, YouTube, Duunitori, Työmarkkinatori, Suno, ManagerSkylark)
  - Credential management for secure authentication

- **Diagnostics & Debugging Tools**:
  - `bin/Aria diag` - Inspects daemon states (port 4444), process tables, active session files, local ManagerSkylark status (port 8080), and current tab state.
  - `bin/Aria page network [filter]` - Inspects DOM performance resource logs (`window.performance.getEntriesByType('resource')`), timing, and payload sizes.
  - `bin/Aria page info / errors` - Quick DOM element counts, document readyState, and JS error inspection.
  - `bin/Aria site managerskylark {status,overview,chat,send,test}` - Specialized integration scraper driving the local Skylark web application and WebChat UI.

- **Core Components**:
  - `BaseAIProvider` - Abstract base for AI providers (e.g., Gemini)
  - `Exceptions` - Error handling and reporting
  - `Logger` - Comprehensive logging throughout the system
  - `Utils` - Utility functions for common operations

## Extension Points
- Add new web scrapers by implementing `GoogleScraper`, `WhatsAppScraper`, etc.
- Create custom plugins via `PluginManager`
- Extend `AriaNavigator` for new navigation strategies
- Add new AI providers inheriting from `BaseAIProvider`
- Implement new safety policies in `SafetyManager`

## Conventions
- Follow `/CODESTYLE.md` for code style and documentation
- Keep all package manifests `.upp` with `file` section starting with `AGENTS.md`
- Use proper U++ naming conventions (PascalCase for classes, camelCase for methods)
- Document all public APIs with Topic++ (`.tpp`) style comments

## Dependencies
- Core: Essential U++ framework components
- WebDriver: Browser automation (Selenium)
- AI Providers: Gemini and other LLM integrations
- Security: Credential management and safety mechanisms

## Where To Start
- Review `Aria.h` for the main orchestrator class
- Examine `AriaNavigator.h/.cpp` for web navigation logic
- Study `ScriptManager.h` for script execution framework
- Look at `SafetyManager.h` for compliance and risk handling
- Check `BaseAIProvider.h` for AI provider interfaces
- Explore `Aria.cpp` for implementation details

## Design Principles
- Declarative task management through script definitions
- Safe execution with built-in safeguards and rollback capabilities
- Extensible plugin architecture for custom integrations
- Comprehensive logging and reporting for audit trails
- Cross-platform compatibility across major web browsers

## Performance Characteristics
- Asynchronous task execution for responsiveness
- Parallel processing of independent subtasks
- Efficient caching of scraped data and site states
- Adaptive resource allocation based on task complexity

## Troubleshooting
- Navigation failures: Check webdriver availability and browser compatibility
- Scraper issues: Verify credentials and site accessibility
- Safety violations: Review `SafetyManager` configuration
- Performance bottlenecks: Monitor task queue and resource utilization

This documentation provides a comprehensive guide to the Aria package, covering its architecture, key components, and extension points for building powerful web automation solutions.