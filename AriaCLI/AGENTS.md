# AriaCLI Package AGENTS Guide

## Scope
- Applies to `uppsrc/AriaCLI` and its entire sub-tree of U++ packages.

## Purpose
- Command-line interface for Aria orchestration and automation
- Provides script execution, task management, and workflow control
- Acts as the gateway for users to interact with the Aria orchestration system
- Integrates with Aria's core orchestration engine and script manager

## Key Areas
- **CLI Interface**: Command-line parsing and argument handling
- **Task Execution**: Running scripts and managing job workflows
- **Orchestration Control**: Starting/stopping Aria jobs and monitoring progress
- **User Interaction**: Help menus, error reporting, and interactive prompts
- **Integration**: Connecting to Aria core services and external tools

## Extension Points
- Add new CLI subcommands for specialized tasks (e.g., `aria-cli run`, `aria-cli monitor`)
- Implement new script runners with custom execution profiles
- Extend job management with priority levels and resource limits
- Add plugin support for third-party automation tools

## Conventions
- Follow `/CODESTYLE.md` for code style and documentation
- Use proper U++ naming conventions (PascalCase for classes, camelCase for methods)
- Include `AGENTS.md` as first file in `.upp` manifest
- Document all public APIs with Topic++ (`.tpp`) style comments
- Use `Separate` and `ReadOnly` options in `.upp` for logical grouping
- Keep package header (`AGENTS.md`) up to date with any structural changes
- Use proper acronym casing for shared components
- Prefer shared implementations over duplicated code patterns
- Follow existing patterns in the codebase

- Follow repo-wide naming rules: `ai/rules/naming-conventions.md`

## Dependencies
- Aria: Core orchestration engine
- CodeEditor: For script editing and management
- Common: Shared utilities and base classes
- TheIDE: Integration with the IDE application

## Where To Start
- Review `AriaCLI.h` for the main CLI class interface
- Examine `AriaCLI.cpp` for command implementation details
- Study `ScriptRunner` for task execution patterns
- Look at `JobManager` for workflow control mechanisms
- Check `HelpSystem` for user interaction components

## Design Principles
- Clear separation between CLI parsing and business logic
- Consistent error handling with user-friendly messages
- Modular design allowing easy addition of new commands
- Performance-focused implementation for fast command execution
- Comprehensive logging for debugging and auditing

