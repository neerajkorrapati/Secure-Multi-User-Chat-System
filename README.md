# Secure Multi-User Chat System

This project is a terminal-based multi-user chat application built in C++ using TCP sockets and SQLite.

The goal was to learn networking, databases, and system design by gradually building a real-time chat server from scratch instead of relying on existing frameworks.

## Features

* Multiple clients connected simultaneously using `select()`
* Public chat (broadcast messaging)
* Private messaging using `/msg`
* User registration and login
* Online user list using `/users`
* Help menu using `/help`
* Chat history retrieval using `/history`
* Join and leave notifications
* Persistent message storage using SQLite
* User account storage using SQLite
* Modular server design with reusable functions

## Commands

```text
/help
/users
/history
/msg <username> <message>
/register <username> <password>
/login <username> <password>
```

## Tech Stack

* C++
* TCP/IP Sockets
* POSIX Networking APIs
* SQLite3
* Multithreading (`std::thread`)
* Linux System Programming

## What I Learned

* Socket programming and TCP communication
* Multi-client server design using `select()`
* Client-server architecture
* SQL fundamentals (CREATE, INSERT, SELECT, WHERE, LIMIT)
* Database integration with SQLite
* Authentication and user management
* Event-driven programming
* Code modularization and refactoring

## Future Improvements

* Password hashing
* End-to-end encryption
* Group chats
* File sharing
* Message timestamps
* GUI client
* Docker deployment

This project was built incrementally to understand how real-world messaging applications work .
