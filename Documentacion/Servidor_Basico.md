# Documentación básica del servidor IRC (ft_irc)

Este documento describe el estado actual del servidor IRC en el repositorio, explica el flujo básico de ejecución, y detalla dónde y cómo implementar la lógica de canales (channels). Está pensado para desarrolladores que quieran entender y ampliar el proyecto sin tocar la lógica existente no relacionada.

## Resumen del proyecto

Estructura relevante:

- `main.cpp` (raíz)
- `Socket/Server.hpp`, `Socket/Server.cpp` — lógica del servidor, gestión de sockets, epoll, clientes y mensajes.
- `Messaging/` — mensajes, parseo, handlers y entidades relacionadas (`Message`, `ParserMessage`, `MessageIn`, `MessageOut`, `Param`, `Channels/`, etc.).
- `Authentication/User.hpp`, `User.cpp` — representación de usuarios y su estado.
- `Documentacion/` — documentación del proyecto (este archivo se añade aquí).

El servidor usa epoll y mantiene vectores paralelos:
- `clients` (vector de `User`) — usuarios conectados.
- `client_fds` (vector de `int`) — file descriptors asociados a cada cliente.
- `messages` (vector de colas) — colas de mensajes a enviar por cliente.
- `servers` (vector de `Channel`) — canales existentes (nomenclatura en código: `servers` para la lista de canales).

Nota: el código adjunto incluye utilidades para añadir usuarios (`addUser`), canales (`addChannel`), y búsquedas por nick/ID (`get_user_by_nick`, `get_user_by_id`, `get_by_channel_name`, `get_by_channel_id`).

## Flujo básico de ejecución

1. `main.cpp` crea una instancia de `Server` y llama a `server.loop(port)` (no incluido en este documento pero presente en `Server.hpp`/`Server.cpp`).
2. El servidor inicializa el socket de escucha y el `epoll` y empieza a aceptar conexiones.
3. Para cada cliente aceptado, se crea/añade un `User` a `clients`, se guarda su FD en `client_fds`, y se crea una cola vacía en `messages`.
4. La recepción y envío de mensajes se gestiona mediante `epoll` y las colas `messages`. Cuando queráis enviar un mensaje a un usuario se usa `add_msg(void *msg, size_t len, bool is_heap, User &receiver)`.
5. La desconexión y limpieza la realiza `disconnect_user` y el destructor de `Server` — estas funciones eliminan el FD de `epoll`, cierran el descriptor y liberan memoria de mensajes pendientes.

## Entidades clave y contratos mínimos

- User
  - Métodos visibles en el código: `get_id()`, `get_nick()`.
  - `get_id()` devuelve `-1` si el usuario no está registrado.

- Channel (en `Messaging/Channels/Channel.hpp` y `Channel.cpp`)
  - Está representado como `Channel` y almacenado en `servers` dentro de `Server`.
  - Métodos esperados: `get_id()`, `get_name()` (según su uso en `Server.cpp`).

- Mensajes
  - `messages` es un vector de `std::queue<std::tuple<void*, size_t, bool>>` donde cada elemento es (puntero, longitud, flag si es memoria en heap).
  - `add_msg` añade mensajes a la cola del usuario receptor.

Contrato operativo (inputs/outputs):
- Input: conexiones TCP entrantes y datos de texto conforme al protocolo IRC.
- Output: paquetes/lines enviados a clientes mediante las colas `messages`.
- Error modes: fallos en `epoll_ctl`, `accept`, `send`/`recv` deben marcar el cliente para desconexión o parar el servidor.

## Dónde y cómo implementar canales (Channels)

Resumen: La implementación de canales debe concentrarse en `Messaging/Channels/` y las interfaces en `Socket/Server.{hpp,cpp}` que ya esperan la existencia de canales (vector `servers`, funciones `addChannel`, `get_by_channel_name`, `get_by_channel_id`). A continuación se detallan pasos concretos.

1. Revisa los ficheros existentes relevantes:
   - `Messaging/Channels/Channel.hpp` y `Messaging/Channels/Channel.cpp` — definición del tipo `Channel`.
   - `Messaging/Channels/Operators/` — operadores de canal (`invite`, `kick`, `mode`, `topic`) ya presentes como punto de partida.
   - `Messaging/ParserMessage.cpp`, `Messaging/Message.cpp` — lugares lógicos donde los comandos IRC se parsean y convierten en acciones.

2. Asegurar la API mínima de `Channel`:
   Implementa (si no están) los métodos utilizados por `Server.cpp`:
   - `size_t get_id() const;` — índice o id único del canal.
   - `std::string get_name() const;` — nombre del canal (`#channel`).
   - Métodos para añadir/quitar miembros: `add_member(User&)`, `remove_member(User&)`.
   - Comprobaciones/flags: `is_operator(User&)`, `has_invite_only()` si es necesario para `mode`.

3. Creación y registro de canales
   - Cuando se parsee `JOIN #channel` en `ParserMessage` o en el handler correspondiente, comprobar si el canal existe con `Server::get_by_channel_name(name)` o iterando sobre `servers`.
   - Si no existe, crear un `Channel` con un nuevo id: `channel.set_id(server.get_max_channel_id()+1)` o gestionar `max_channel_id` en `Server`. Actualmente `Server` expone `max_channel_id`, inicializado a 0: usarlo para asignar IDs y actualizarlo.
   - Añadir el canal al servidor con `Server::addChannel(newChannel)`.

4. Asociar usuarios y fds
   - Los `User` están en `clients`; cuando un usuario se une a un canal, actualizar la estructura interna del `Channel` (lista de miembros) y, si procede, enviar notificaciones a todos los miembros añadiendo mensajes a sus colas via `Server::add_msg`.

5. Operadores e implementación de comandos
   - Los operadores ya presentes (`invite`, `kick`, `mode`, `topic`) deberían integrarse con el parser/handlers. Asegura que el handler recibe el `Server&` y `Channel&` o `User&` según sea necesario.
   - Añade tests unitarios en `Testing/` para `JOIN`, `PART`, `PRIVMSG` en contexto de canales.

## Ejemplo de flujo JOIN (alto nivel)

1. Cliente envía: `JOIN #foo`.
2. `ParserMessage` produce un objeto `Message`/`Command` con verb `JOIN` y params.
3. Se invoca el handler de `JOIN` en `fnHandlers.cpp` (o similar). El handler:
   - Valida parámetros y permisos.
   - Usa `server.get_by_channel_name("#foo")` para localizar el canal; si no existe, crea uno y llama a `server.addChannel()`.
   - Llama a `channel.add_member(user)` y luego envía `RPL_TOPIC`/`RPL_NAMREPLY` según el protocolo IRC, usando `server.add_msg()` para notificar a los miembros.

## Puntos concretos para editar/añadir (sin modificar código existente aún)

- Revisar/implementar en `Messaging/Channels/Channel.hpp`:
  - Atributos: id, name, vector<User> members, topic, modes (bitflags), invite_list.
  - Métodos: `add_member`, `remove_member`, `broadcast(message)`, `is_member`, `get_members_list`.

- Integrar handlers:
  - En `Messaging/fnHandlers.cpp` añadir/añadir llamadas a funciones de `Channel` para `JOIN`, `PART`, `NAMES`, `PRIVMSG` (hacia canal).

- Tests:
  - Añadir `Message_test.cpp` o crear tests nuevos en `Testing/` para validar parseo y efectos de `JOIN`/`PART`.

## Ejemplo de firma de funciones/contratos recomendados

- Server::addChannel(Channel &ch) — ya existe; asegurar que el canal queda con un id consistente.
- Server::add_msg(void *msg, size_t len, bool is_heap, User &receiver) — ya existe; usarla para notificaciones.
- Channel::broadcast(Server &server, const std::string &raw_msg) — iterar miembros y llamar `server.add_msg()` para cada miembro.

## Edge cases y consideraciones

- Consistencia de indices: `Server` usa vectores paralelos (`clients`, `client_fds`, `messages`) — las operaciones de borrado reindexan; asegurar que los objetos guardan IDs estables o que se usan IDs y búsquedas por `get_id()` cuando sea necesario.
- Concurrencia/epoll: las operaciones con `epoll_ctl` pueden fallar y deben provocar limpieza segura.
- Gestión de memoria: `messages` contiene tuples con punteros; si el flag `is_heap` está activado el receptor de mensajes debe liberar con `delete[]` cuando procese o descarte los mensajes.

## Siguientes pasos sugeridos

1. Implementar o revisar `Messaging/Channels/Channel.hpp` y `Channel.cpp` para exponer métodos sugeridos.
2. Integrar handlers de comandos de canal dentro de `Messaging/fnHandlers.cpp` y `Messaging/ParserMessage.cpp`.
3. Añadir pruebas unitarias básicas que cubran `JOIN`, `PART`, `PRIVMSG` y `NAMES`.
4. Documentar las decisiones de diseño en `Documentacion/IRC Canales.md` si se requieren reglas especiales de modos/privilegios.

---

Archivo generado por el desarrollador: Documentacion/Servidor_Basico.md

Si quieres, puedo:
- Añadir ejemplos de código (funciones esqueleto) para `Channel` y handlers.
- Crear tests iniciales en `Testing/`.
- Generar una checklist de tareas para implementar las funciones mencionadas.

Indica cuál de las opciones prefieres y lo hago a continuación.
