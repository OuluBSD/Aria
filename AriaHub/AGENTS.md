# AriaHub Package AGENTS Guide

## Scope
- Applies to `uppsrc/AriaHub` and its entire sub-tree of U++ packages.

## Purpose
- Central hub for Aria orchestration and automation services
- Provides a unified interface for managing multiple Aria instances
- Enables distributed task execution across multiple nodes
- Acts as the coordination point for all Aria-related services

## Key Areas
- **Hub Management**: Central coordination and service registry
- **Node Management**: Registration and monitoring of Aria nodes
- **Task Distribution**: Load balancing and task assignment
- **Service Discovery**: Dynamic service registration and lookup
- **Communication Layer**: Inter-node messaging and synchronization

## Extension Points
- Add new hub services by extending `HubService` base class
- Implement custom node types for specialized execution environments
- Add new task distribution algorithms
- Extend communication protocols with new transport mechanisms
- Integrate with external service registries

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
- AriaCLI: Command-line interface
- Common: Shared utilities and base classes
- TheIDE: Integration with the IDE application

## Where To Start
- Review `AriaHub.h` for the main hub class interface
- Examine `AriaHub.cpp` for hub implementation details
- Study `NodeManager` for node registration and monitoring
- Look at `TaskDistributor` for task assignment algorithms
- Check `ServiceRegistry` for service discovery mechanisms

## Design Principles
- High availability with automatic failover
- Scalable architecture supporting multiple nodes
- Loose coupling between hub and node components
- Consistent state management across all services
- Comprehensive monitoring and logging

