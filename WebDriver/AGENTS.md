# WebDriver Package AGENTS Guide

## Purpose
This package provides WebDriver functionality for browser automation, adapted from the webdriverxx library.

## Structure
The package follows the structure of the original webdriverxx library with adaptations for the U++ framework.

## Key Components
- WebDriver: Main class for interactions with a server
- Session: Represents a browser session
- Element: Represents DOM elements
- Capabilities: Configuration for browser sessions
- Browser-specific classes: Chrome, Firefox, IE, Phantom

## Conventions
- All classes are wrapped in the UPP namespace
- Follow U++ naming conventions
- Use U++ types where appropriate

## Dependencies
- Core: Basic U++ functionality
- Core: the only package dependency (raw sockets via `ws2_32` on WIN32)