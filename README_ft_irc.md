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

**ft_irc** es un **servidor IRC** implementado en C++ que sigue el protocolo cliente-servidor descrito en las RFC clásicas.
Permite conexiones simultáneas, creación/gestión de canales, mensajería entre usuarios y aplicación de modos de canal.

**Objetivo:** reproducir el comportamiento esperado por clientes IRC reales (HexChat, irssi, WeeChat, netcat) respetando el formato de mensajes, numerics, y reglas de canal.

---

## 🧠 Quickstart (TL;DR)

```bash
make
./ircserv 6667 pass123
```

Conexión rápida con `nc`:

```bash
nc 127.0.0.1 6667
```

Registro mínimo:

```diff
+ PASS pass123
+ NICK wiz
+ USER wiz 0 * :Wizard User
```

---

## 🛠️ Instructions

### 1) Compile

```bash
make
```

Opcionales:

```bash
make clean
make fclean
make re
```

### 2) Run

```bash
./ircserv <port> <password>
```

Ejemplo:

```bash
./ircserv 6667 pass123
```

### 3) Connect (client real)

- **Server:** `127.0.0.1`
- **Port:** `6667`
- **Password:** `pass123`

---

## 🎮 Supported Commands

> Formato: `COMANDO [args]` — *qué hace*.

| Command | Syntax | What it does |
|---|---|---|
| **PASS** | `PASS <password>` | Define la contraseña de conexión. Debe enviarse antes de `NICK/USER`. |
| **NICK** | `NICK <nick>` | Set / change nickname. |
| **USER** | `USER <user> <mode> <unused> :<realname>` | Completa el registro del usuario. |
| **JOIN** | `JOIN <#channel> [key]` | Entra (o crea) un canal. Con `key` si el canal tiene `+k`. |
| **PRIVMSG** | `PRIVMSG <recipient> :<text>` | Envía mensaje a usuario o canal. |
| **PING** | `PING <server>` | Keep-alive. Debe contestarse con `PONG`. |
| **PONG** | `PONG <server>` | Respuesta a `PING`. |
| **QUIT** | `QUIT :<reason>` | Sale del servidor con razón opcional. |
| **KICK** | `KICK <#channel> <user> :<reason>` | Expulsa a un usuario del canal (requiere ops). *(Si tu proyecto aún está “TODO”, deja esto como “WIP”)* |
| **INVITE** | `INVITE <user> <#channel>` | Invita a un usuario a un canal (solo miembros; si `+i`, solo ops). |
| **TOPIC** | `TOPIC <#channel> [ :topic ]` | Lee o cambia el topic. Si el canal tiene `+t`, solo ops pueden cambiarlo. *(Si está “TODO”, márcalo WIP)* |
| **MODE** | `MODE <#channel> <modes...>` | Consulta/cambia modos del canal. |

---

## 🧩 Channel Modes (implemented)

Modos típicos que aparecen en ft_irc:

- `+i` / `-i` → Invite-only
- `+t` / `-t` → Topic solo por operadores
- `+k <key>` / `-k` → Password del canal
- `+o <nick>` / `-o <nick>` → Dar/quitar operador
- `+l <limit>` / `-l` → Límite de usuarios

Ejemplos:

```diff
+ MODE #canal +i
+ MODE #canal +k superkey
+ MODE #canal +l 10
+ MODE #canal +o wiz
- MODE #canal -k
- MODE #canal -l
```

---

## 🧪 Examples

### Create & chat

```diff
+ JOIN #general
+ PRIVMSG #general :hola chat 😎
```

### Invite-only flow

```diff
+ MODE #general +i
+ INVITE rata_ #general
```

### Topic

```diff
+ TOPIC #general :Foro castizo
```

---

## 📚 Resources

### RFCs (golden references)
- RFC 2812 — [*IRC Client Protocol*]
- RFC 2811 — *IRC Channel Management*
- RFC 1459 — *Classic IRC Protocol*

### Tools used.

netcat (nc) - Simple TCP/IP client for testing
HexChat - Graphical IRC client for testing
Irssi - Terminal IRC client
WeeChat - Extensible IRC client
Valgrind - Memory leak and memory error detection

### Practical references
- Libera.Chat docs (client behavior / real-world quirks)

### 🤖 How AI was used
Se utilizó IA (ChatGPT / Copilot) **como apoyo**, específicamente para:
- Ayuda a determinar que componentes se han de incluir en el README.
- Procesamiento de la documentación pertinente al proyecto (RFCs, bogs, etc.)

**No** se utilizó IA para “pegar” una solución completa sin entenderla: toda sugerencia se revisó y adaptó manualmente por el equipo.

---

<p align="center">
  <img alt="made with love" src="https://img.shields.io/badge/made%20with-CRLF%20%26%20pain-ef4444?style=for-the-badge">
</p>
