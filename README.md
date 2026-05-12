# ft_irc

An IRC server built from scratch in C++, implementing the IRC protocol with support for channels, operators, and real-time messaging.

Team project at Hive Helsinki (42 School) – forked from the original team repository.

## Team Contributions

Three-person project with distinct ownership areas across server core, command logic, and command parsing.

### My Work (Server Core & Channel Commands)

- **Server architecture** – IRC server setup from scratch, non-blocking I/O with poll()
- **Connection lifecycle** – registration flow, idle detection, ping/pong timeout, and graceful disconnect
- **Buffered I/O** – per-client read/write buffers routed through poll(), no blocking on partial sends
- **Flood protection** – per-client buffer limits and registration timeouts
- **Numeric reply system** – formatting, error codes, and channel broadcast
- **Auth commands** – PASS, NICK, CAP, PING/PONG
- **Channel commands** – JOIN, TOPIC, KICK, PRIVMSG, QUIT, WHO

### Emilia's Work

- Message parsing engine
- USER command
- Port validation

### Abdulbaki's Work

- MODE command
- PART command
- INVITE command

## Requirements

- C++ compiler (g++/clang++)

## Installation

```bash
git clone https://github.com/KozhInna/42_irc.git ft_irc
cd ft_irc
make
```

## Usage

```bash
./ircserv 6667 <password>
```

Connect with any IRC client (e.g. irssi, or netcat for testing):

```bash
nc localhost 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice Smith
JOIN #general
PRIVMSG #general :hello!
```