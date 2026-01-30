# Flujo de ejecución del servidor IRC

Esta documentación resume, en español, el flujo de ejecución del servidor y señala las funciones/archivos clave provistos en el proyecto.

## Resumen general
- Componentes principales:
  - Server (srcs/Socket/Server.cpp y server_loop.cpp): gestión de sockets, epoll, clientes, colas de mensajes y lógica principal.
  - router (srcs/Messaging/router.cpp): parseo de mensajes IRC entrantes y despacho a los handlers correspondientes.
  - Handlers de comandos (srcs/Messaging/commands.cpp): implementaciones de comandos (PASS, NICK, USER, PING, PONG, QUIT, HELP, etc.).
  - User (Include/User.hpp, srcs/Authentication/User.cpp): estado de cada cliente (nick, username, realname, id, buffer de mensaje parcial).
  - Channel (Include/Channels/Channel.hpp): manejo y broadcast en canales (utilizado por handlers).
- Mensajería interna: el servidor mantiene una cola de mensajes por usuario (messages); `add_msg` añade mensajes a la cola; `handle_write_event` extrae y escribe al socket.

## Flujo detallado de ejecución

1) Arranque del servidor
- `Server::Server(passw)` inicializa variables internas (listas vacías, flags, contraseña).
- `Server::loop(PORT)`:
  - Registra señales (SIGINT, SIGQUIT, etc.).
  - Llama a `setup_sockfd(PORT)` para crear `sockfd` (socket, bind, listen, O_NONBLOCK).
  - Crea `epollfd` y registra `sockfd` (aceptar conexiones) y `stdin` (fd 0) para comandos de operador.
  - Entra en el bucle principal: `epoll_wait(events, MAX_EVENTS, timeout)`.

2) Aceptación de nuevos clientes
- Evento en `sockfd` → `Server::handle_event` hace `accept()`:
  - Crea `new_client_fd` y añade estructuras: `clients`, `client_fds`, `last_pong_time`, `messages`.
  - Registra `new_client_fd` en epoll con `EPOLLIN | EPOLLOUT`.
  - Envía mensaje inicial de bienvenida con `add_msg` al nuevo cliente (estado "unregistered").

3) Lectura desde sockets (EPOLLIN)
- En `handle_read_event(fd)`:
  - Lee en bucle bloques de `READ_SIZE` y concatena en `data`.
  - Opcionalmente convierte `\n` en `\r\n` si `replace_LF_to_CRLF` está activo.
  - Si `fd == 0` (stdin): interpreta comandos de consola (quit, toggles) y retorna.
  - Obtiene `User *sender` con `get_user_by_fd(fd)`.
  - Llama `sender->msg_sent(data)` para ensamblar mensajes terminados en CRLF:
    - `User::msg_sent` mantiene `current_message` entre llamadas y devuelve vector de mensajes completos.
  - Para cada mensaje completo: llama `route_message(msg, *sender, sender_index)` (dispatcher).

4) Sanitización y parseo (router)
- `router::operator()(message, server, sender)`:
  - Rechaza mensajes > 512 bytes.
  - Llama `sanitize(message)` (quita caracteres no imprimibles y normaliza espacios).
  - `split(sanitized, ' ')` tokeniza por espacios.
  - Reconoce `prefix` si el primer token empieza por `:` y lo elimina de `argv`.
  - Busca el índice del comando comparando con `command_string[]`.
  - Validaciones:
    - Comando desconocido → envía NOTICE con "command not found".
    - Si no se pasó PASS aún y el comando no es PASS/QUIT/HELP/PONG → pide PASS.
    - Si PASS fue aceptado pero falta registro (NICK+USER) → pide NICK y USER.
  - Construye `args.raw_args` y `args.argv` y llama al handler asociado `fun[func_i](args, server, sender)`.

5) Handlers de comando (commands.cpp)
- Macros útiles:
  - `send_back(msg)`: convierte y manda respuesta del servidor usando `server.add_msg`.
  - `notice_back(msg)`: envía NOTICE al nick del emisor.
  - `send_return(msg)`: envía y retorna del handler.
  - `register()`: envía los mensajes RPL (001..004) y marca `sender` como registrado.
  - `suppr()`: placeholder sin efecto funcional (mantiene compatibilidad del código existente).
- Ejemplos:
  - `PASS_fn`: valida contraseña, llama `sender.passwd_match_pop(true)` en caso correcto.
  - `NICK_fn`: valida disponibilidad, actualiza nick, si ya registrado notifica a canales; si USER ya está seteado y es la primera vez que pone NICK, llama `register()`.
  - `USER_fn`: valida parámetros, setea `username`, `hostname`, y `realname`; si `nick` ya estaba presente, llama `register()`.
  - `PING_fn` / `PONG_fn`: `PING_fn` responde con `PONG` si el usuario está registrado; `PONG_fn` actualiza el tiempo de respuesta (`Server::set_pong_time`).
  - `QUIT_fn`: desconecta al usuario (`server.disconnect_user(...)`).

6) Colas de salida y envío (EPOLLOUT)
- `Server::handle_write_event(fd)`:
  - Localiza índice del usuario por `fd`.
  - Si hay mensajes en `messages[user_i]`, extrae el siguiente y hace `write(fd, next_msg.data(), next_msg.size() + 1)`.
  - `add_msg` normalmente asegura que los mensajes terminen en CRLF.

7) Ping activo y timeouts
- En el bucle principal, si `send_pings_actively` y ha pasado `PING_SEPARATION_S` desde `last_ping_time`, llama `Server::send_pings()`:
  - Para cada cliente: si `time(NULL) - last_pong_time[i] > USER_TIMEOUT_S` → desconecta por timeout.
  - Si el cliente respondió al ping anterior (`last_pong_time[i] > last_ping_time`) → el servidor encola otro `PING` para ese cliente.
  - Actualiza `last_ping_time`.
- `PONG_fn` actualiza `last_pong_time` a través de `Server::set_pong_time(user_id)`.

8) Gestión y búsqueda de usuarios
- Métodos auxiliares en `Server`:
  - `get_user_index_by_fd(fd)`, `get_user_by_fd(fd)` para mapear fd → User.
  - `get_user_by_nick(nick)`, `get_user_index_by_id(id)` para búsquedas por nick/id.
  - `add_msg` localiza cola por `receiver.get_id()` y hace push del mensaje.

## Puntos importantes y observaciones
- `User::msg_sent` reensambla mensajes fragmentados por lectura parcial y depende de CRLF para delimitar mensajes completos.
- Las macros en `commands.cpp` (especialmente `register()` y `send_back`) facilitan respuestas rápidas, pero concatenan cadenas de forma manual; tener cuidado con prefijos y formatos IRC.
- `suppr()` es un placeholder que no altera estado; varios handlers están aún sin implementar (JOIN, PRIVMSG, KICK, INVITE, TOPIC, MODE).
- `handle_write_event` usa `write(..., size()+1)` (incluye un byte más) — revisar si esto es intencional (posible bug si se escribe byte extra).
- Al desconectar usuarios, `Server::disconnect_user` elimina su FD de epoll, cierra el socket y remueve entradas en vectores paralelos (`client_fds`, `last_pong_time`, `messages`, `clients`).

---

Archivo(s) relacionados (ejemplos):
- srcs/Socket/Server.cpp
- srcs/Socket/server_loop.cpp
- srcs/Messaging/router.cpp
- srcs/Messaging/commands.cpp
- srcs/Authentication/User.cpp
- Include/User.hpp
- Include/Channels/Channel.hpp

Este documento pretende servir como guía para entender el flujo de ejecución y los puntos de integración entre módulos. Para ampliar, se pueden agregar diagramas de secuencia o ejemplos de mensajes IRC y respuestas esperadas.
