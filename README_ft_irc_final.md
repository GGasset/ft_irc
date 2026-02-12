_This project has been created as part of the 42 curriculum by layala-d, alvmoral, ggasset-._

<p align="center">
  <img alt="IRC" src="https://img.shields.io/badge/IRC-server-7C3AED?style=for-the-badge&logo=probot&logoColor=white">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-98-2563EB?style=for-the-badge&logo=c%2B%2B&logoColor=white">
  <img alt="42" src="https://img.shields.io/badge/42-curriculum-111827?style=for-the-badge">
</p>

<p align="center">
  <img alt="RFC 2812" src="https://img.shields.io/badge/RFC-2812-10B981?style=flat-square">
  <img alt="RFC 2811" src="https://img.shields.io/badge/RFC-2811-10B981?style=flat-square">
  <img alt="Sockets" src="https://img.shields.io/badge/sockets-TCP-F59E0B?style=flat-square">
  <img alt="I/O" src="https://img.shields.io/badge/event--driven-epoll-EC4899?style=flat-square">
</p>

---

## ✨ Description

**ft_irc** is an **IRC server** written in C++ that follows the classic IRC client-server protocol described in the RFCs.

- Supports multiple simultaneous clients ✅  
- Channel creation and management ✅  
- User-to-user and channel messaging ✅  
- Channel modes support ✅  

**Goal:** behave as a real IRC server so it can be used with standard clients (HexChat, irssi, WeeChat, netcat), respecting message formatting, numerics, and channel rules.

---

## 🎨 Legend


**IRC commands (blue):**  
![](https://img.shields.io/badge/COMMAND-1e40af?style=flat-square)

**Parameters (gray):**  
![](https://img.shields.io/badge/PARAM-6b7280?style=flat-square)

**Terminal binaries (PowerShell-like yellow):**  
![](https://img.shields.io/badge/BINARY-facc15?style=flat-square)

**Pink `@` and matching user/host tones:**  
![](https://img.shields.io/badge/@-ff4fd8?style=flat-square) ![](https://img.shields.io/badge/user%2Fhost-f9a8d4?style=flat-square)

**Numerics:** replies = green (`+`), errors = red (`-`) via `diff`.

---

## 🧠 Quickstart (TL;DR)

```bash
make
```

![](https://img.shields.io/badge/./ircserv-facc15?style=flat-square&logo=windows-terminal&logoColor=black) `6667` `pass123`

Quick connect with netcat:

![](https://img.shields.io/badge/nc-facc15?style=flat-square&logo=gnome-terminal&logoColor=black) `127.0.0.1` `6667`

---

## 🔐 Register flow (PASS → NICK → USER)

> The following is a minimal registration sequence with **expected output** (example).  

### 1) PASS

**Client ➜ server**  
![](https://img.shields.io/badge/PASS-1e40af?style=flat-square) ![](https://img.shields.io/badge/1234-6b7280?style=flat-square)

**Server ➜ client**
```diff
+ NOTICE unregistered
+ Welcome! Go ahead and login with PASS [passw].
+ You may also write HELP to get a list of commands
```

### 2) NICK

**Client ➜ server**  
![](https://img.shields.io/badge/NICK-1e40af?style=flat-square) ![](https://img.shields.io/badge/Alvaro-6b7280?style=flat-square)

**Server ➜ client (yellow)**
```diff
+NOTICE Alvaro Nick set to: Alvaro
```

### 3) USER

**Client ➜ server**  
![](https://img.shields.io/badge/USER-1e40af?style=flat-square) ![](https://img.shields.io/badge/a%200%20*%20:aa-6b7280?style=flat-square)

**Server ➜ client**
```diff
+ NOTICE Alvaro Username set to: a. Hostname set to: 0. Realname set to: aa
+ ::Arepa_de_makako@localhost  001 Welcome to the Internet Relay Network Alvaro!a@0
+ ::Arepa_de_makako@localhost  002 Your host is :Arepa_de_makako@localhost, running version 42
+ ::Arepa_de_makako@localhost  003 This server was created today, I bet ;)
+ ::Arepa_de_makako@localhost  004 :Arepa_de_makako@localhost 42 operator normal +i +k +l +o +t
```

### Bonus: prefix styling (pink `@`)

- `::Arepa_de_makako` (cyan) + `@` (pink) + `localhost` (light pink)

Example:

![](https://img.shields.io/badge/::Arepa_de_makako-06b6d4?style=flat-square)
![](https://img.shields.io/badge/@-ff4fd8?style=flat-square)
![](https://img.shields.io/badge/localhost-f9a8d4?style=flat-square)
![](https://img.shields.io/badge/001-22c55e?style=flat-square)
`Welcome to the Internet Relay Network Alvaro!a@0`

---

## 🛠️ Instructions

### 1) Compile

```bash
make
```

Optional:

```bash
make clean
make fclean
make re
```

### 2) Run

![](https://img.shields.io/badge/./ircserv-facc15?style=flat-square&logo=windows-terminal&logoColor=black) `<port>` `<password>`

Example:

![](https://img.shields.io/badge/./ircserv-facc15?style=flat-square&logo=windows-terminal&logoColor=black) `6667` `pass123`

### 3) Connect (real IRC client)

- **Server:** `127.0.0.1`
- **Port:** `6667`
- **Password:** `pass123`

---

## 🎮 Supported Commands

> Format: blue command + gray parameters.

| Command | Syntax | What it does |
|---|---|---|
| **PASS** | ![](https://img.shields.io/badge/PASS-1e40af?style=flat-square) ![](https://img.shields.io/badge/<password>-6b7280?style=flat-square) | Sets the connection password. Must be sent before `NICK/USER`. |
| **NICK** | ![](https://img.shields.io/badge/NICK-1e40af?style=flat-square) ![](https://img.shields.io/badge/<nick>-6b7280?style=flat-square) | Sets / changes nickname. |
| **USER** | ![](https://img.shields.io/badge/USER-1e40af?style=flat-square) ![](https://img.shields.io/badge/<user>%20<mode>%20<unused>%20:<realname>-6b7280?style=flat-square) | Completes user registration. |
| **JOIN** | ![](https://img.shields.io/badge/JOIN-1e40af?style=flat-square) ![](https://img.shields.io/badge/<%23channel>%20[key]-6b7280?style=flat-square) | Joins (or creates) a channel. Uses `key` if `+k` is set. |
| **PRIVMSG** | ![](https://img.shields.io/badge/PRIVMSG-1e40af?style=flat-square) ![](https://img.shields.io/badge/<recipient>%20:<text>-6b7280?style=flat-square) | Sends a private message to a user or channel. |
| **PING** | ![](https://img.shields.io/badge/PING-1e40af?style=flat-square) ![](https://img.shields.io/badge/<sender>-6b7280?style=flat-square) | Keep-alive. Must be answered with `PONG`. |
| **PONG** | ![](https://img.shields.io/badge/PONG-1e40af?style=flat-square) | Reply to `PING`. |
| **QUIT** | ![](https://img.shields.io/badge/QUIT-1e40af?style=flat-square) ![](https://img.shields.io/badge/<reason>-6b7280?style=flat-square) | Disconnects from server with optional reason. |
| **KICK** | ![](https://img.shields.io/badge/KICK-1e40af?style=flat-square) ![](https://img.shields.io/badge/TODO-6b7280?style=flat-square) | Kicks a user from a channel (requires operator privileges). |
| **INVITE** | ![](https://img.shields.io/badge/INVITE-1e40af?style=flat-square) ![](https://img.shields.io/badge/<user>%20<%23channel>-6b7280?style=flat-square) | Invites a user to a channel. |
| **TOPIC** | ![](https://img.shields.io/badge/TOPIC-1e40af?style=flat-square) ![](https://img.shields.io/badge/TODO-6b7280?style=flat-square) | Gets/sets channel topic. |
| **MODE** | ![](https://img.shields.io/badge/MODE-1e40af?style=flat-square) ![](https://img.shields.io/badge/<channel>%20<mode>-6b7280?style=flat-square) | Queries/changes channel modes. |

---

## 🧩 Channel Modes (implemented)

- `+i` / `-i` → Invite-only channel
- `+t` / `-t` → Only channel operators can change TOPIC
- `+k <key>` / `-k` → Set/remove channel key (password)
- `+o <nick>` / `-o <nick>` → Give/take operator privilege
- `+l <limit>` / `-l` → Set/remove user limit

Examples:

```diff
+ MODE #tea +i
+ MODE #tea +k winwardium_leviosa
+ MODE #tea +l 10
+ MODE #tea +o alice
- MODE #tea -k
- MODE #tea -l
```

---

## 🧪 Examples (placeholders)

> Server responses below are **invented placeholders** — paste your real ones later.

### JOIN + TOPIC

```diff
+ :alice!alice@localhost JOIN #tea
```

### PRIVMSG

**Client ➜ server**  
![](https://img.shields.io/badge/PRIVMSG-1e40af?style=flat-square) ![](https://img.shields.io/badge/%23tea%20:Hello%20everyone%20from%20Alice-6b7280?style=flat-square)

```diff
+ :alice!alice@localhost PRIVMSG #tea :Hello everyone from Alice
+ :alice@alice PRIVMSG :Hello eberyone form Alice
```

### TOPIC

**Client ➜ server**  
![](https://img.shields.io/badge/TOPIC-1e40af?style=flat-square) ![](https://img.shields.io/badge/%23tea%20:Infinite%20tea%20table-6b7280?style=flat-square)

```diff
+ :alice!alice@localhost TOPIC #tea :Infinite tea table
```

### PING / PONG

```diff
+ PING alice
+ PONG irc.test :alice
```

### PART

```diff
+ :alice!alice@localhost PART #tea :I will be back soon
```

### QUIT

```diff
+ QUIT :See you later
+ ERROR :Closing Link: alice[localhost] (See you later)
```

### Numeric examples (green replies / red errors)

```diff
+ :Arepa_de_makako@localhost 001 alice :Welcome to the Internet Relay Network alice!alice@localhost
+ :Arepa_de_makako@localhost 332 alice #tea :Infinite tea table
- :Arepa_de_makako@localhost 401 alice #nope :No such nick/channel
- :Arepa_de_makako@localhost 464 * :Password incorrect
```

---

## 📚 Resources

### RFCs (golden references)
- RFC 2812 — *IRC Client Protocol*
- RFC 2811 — *IRC Channel Management*
- RFC 1459 — *Classic IRC Protocol*

### Practical references
- Libera.Chat docs (real-world IRC behavior)

### 🤖 How AI was used
AI tools (ChatGPT / Copilot) were used **as support**, specifically for:
- Documentation writing (README structure, command descriptions, examples)
- Reviewing protocol edge cases (message format, replies, numerics)
- Occasional debugging assistance (parsing flows, logic checks)

AI was **not** used to blindly paste a full solution: every suggestion was reviewed and adapted by the team.

---

<p align="center">
  <img alt="made with love" src="https://img.shields.io/badge/made%20with-CRLF%20%26%20pain-ef4444?style=for-the-badge">
</p>
