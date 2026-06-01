# Secure Multi-User Chat System

This is a networking project I'm building to learn how real-time messaging systems work under the hood.

The idea started as a simple TCP client-server program and is gradually being expanded into a multi-user chat system with authentication, message history, and database support.

## What I'm Learning

* Linux socket programming
* TCP/IP communication
* Client-server architecture
* Handling multiple clients simultaneously
* Event-driven programming using `select()` and later `epoll()`
* Database integration with SQLite
* Designing custom communication protocols

## Current Features:

* TCP client-server communication
* Multiple client connections
* Event-driven server using `select()`
* Client connect/disconnect handling

## Planned Features

* Broadcast messaging
* Private messaging
* User login/register
* SQLite database integration
* Message history
* Online users list
* Heartbeat timeout detection
* Non-blocking sockets
* `epoll()` implementation

## Tech Stack

* C++17
* Linux Sockets
* select()
* SQLite3
* Git & GitHub
* WSL Ubuntu

## Project Structure

```text
secure-multi-user-chat-system/

├── client/
├── server/
├── database/
├── docs/
├── tests/
├── README.md
└── Makefile
```

## Why I Built This

I wanted a project that would help me understand networking and systems programming beyond classroom examples. Instead of using frameworks, I'm building the core pieces myself to learn how chat applications and real-time servers actually work.

## Progress

* [x] Basic TCP client-server communication
* [x] Multi-client support using `select()`
* [ ] Broadcast messaging
* [ ] Authentication
* [ ] SQLite integration
* [ ] Message history
* [ ] epoll() migration
