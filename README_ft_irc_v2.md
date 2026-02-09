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

### Registro mínimo (con salida esperada)

> Los ejemplos de salida están escritos **tal cual** los imprime nuestro servidor (colores incluidos cuando aplican).
> Si tu terminal no soporta ANSI, verás los escapes `\x1b[...]`.

#### 1) PASS

**Cliente ➜ servidor**
```irc
PASS 1234
```

**Servidor ➜ cliente**
```irc
NOTICE unregistered
Welcome! Go ahead and login with PASS [passw].
You may also write HELP to get a list of commands
```

#### 2) NICK

**Cliente ➜ servidor**
```irc
NICK Alvaro
```

**Servidor ➜ cliente (🟡 amarillo)**
```irc
\x1b[33mNOTICE Alvaro Nick set to: Alvaro\x1b[0m
```

#### 3) USER

**Cliente ➜ servidor**
```irc
USER a 0 * :aa
```

**Servidor ➜ cliente**
```irc
NOTICE Alvaro Username set to: a. Hostname set to: 0. Realname set to: aa
::Arepa_de_makako@localhost  001 Welcome to the Internet Relay Network Alvaro!a@0
::Arepa_de_makako@localhost  002 Your host is :Arepa_de_makako@localhost, running version 42
::Arepa_de_makako@localhost  003 This server was created today, I bet ;)
::Arepa_de_makako@localhost  004 :Arepa_de_makako@localhost 42 operator normal +i +k +l +o +t
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

## 🧪 Examples (copy/paste)

> **Nota:** las respuestas del servidor aquí están **inventadas** (placeholders) para que luego pegues las tuyas con el formato exacto de tu implementación.

### 1) JOIN (con TOPIC si existe)

```irc
:alice!alice@localhost JOIN #tea
:irc.test 332 alice #tea :Mesa del té infinita        (si hay topic)
:irc.test 353 alice = #tea :@alice bob charlie
:irc.test 366 alice #tea :End of /NAMES list.
```

### 2) PRIVMSG a un canal

```irc
PRIVMSG #tea :Hola a todos desde Alice
```

*(respuesta típica: el mensaje “rebotado” al canal con prefijo del emisor)*

```irc
:alice!alice@localhost PRIVMSG #tea :Hola a todos desde Alice
```

### 3) TOPIC (cambiar topic)

```irc
TOPIC #tea :Mesa del té infinita
```

```irc
:alice!alice@localhost TOPIC #tea :Mesa del té infinita
```

### 4) PING / PONG

```irc
PING alice
```

```irc
PONG irc.test :alice
```

*(placeholder extra de servidor, por si quieres añadir “lag pong” o similares)*

```irc
:irc.test NOTICE alice :PONG received (placeholder)
```

### 5) PART

```irc
PART #tea :Me voy un momento
```

```irc
:alice!alice@localhost PART #tea :Me voy un momento
```

### 6) QUIT

```irc
QUIT :Hasta luego
```

```irc
ERROR :Closing Link: alice[localhost] (Hasta luego)   (placeholder)
```

---

## 📚 Resources

### RFCs (golden references)
- RFC 2812 — [*IRC Client Protocol*](https://datatracker.ietf.org/doc/html/rfc2812)
- RFC 2813 — [*IRC Server Protocol*](https://datatracker.ietf.org/doc/html/rfc2813)
- RFC 2811 — [*IRC Channel Management*](https://datatracker.ietf.org/doc/html/rfc2811)
- RFC 1459 — [*Classic IRC Protocol*](https://datatracker.ietf.org/doc/html/rfc1459)

### Tools used
- netcat (nc) - Simple TCP/IP client for testing
- HexChat - Graphical IRC client for testing
- Irssi - Terminal IRC client
- WeeChat - Extensible IRC client
- Valgrind - Memory leak and memory error detection

### Practical references
- Libera.Chat docs (client behavior / real-world quirks)


### 🤖 How AI was used
Se utilizó IA (ChatGPT / Copilot) **como apoyo**, específicamente para:
- Redacción de documentación (README, notas de comandos, ejemplos).
- Revisión de edge cases del protocolo (formato de mensajes, replies, numerics).
- Ayuda puntual en depuración (lectura de logs, validación de parsing y flujo de registro).

**No** se utilizó IA para “pegar” una solución completa sin entenderla: toda sugerencia se revisó y adaptó manualmente por el equipo.

---

<p align="center">
  <img alt="made with love" src="https://img.shields.io/badge/made%20with-CRLF%20%26%20pain-ef4444?style=for-the-badge">
</p>
