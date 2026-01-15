# Flujo de ejecución detallado del servidor IRC (ft_irc)

Este documento explica paso a paso el flujo de ejecución del programa tal y como está implementado ahora, y señala con precisión los puntos donde se pueden integrar las funcionalidades de canales (channels). No contiene código; sólo descripciones operativas pensadas para desarrolladores.

## Objetivo

Proveer un mapa preciso del runtime: qué sucede desde que el servidor arranca hasta que procesa comandos de usuario y cómo y dónde intervenir para implementar o extender la lógica de canales.

## Componentes principales

- `main.cpp`: arranque del programa y creación de la instancia `Server`.
- `Socket/Server.*`: gestión de sockets, epoll, lista de clientes (`clients`), fds (`client_fds`), colas de salida (`messages`) y lista de canales (`servers`).
- `Authentication/User.*`: representación del usuario y estado (nick, id, etc.).
- `Messaging/`: parseo de mensajes, objetos `Message`, `ParserMessage`, functions handlers (`fnHandlers.*`) y el subdirectorio `Channels/`.

## Flujo de arranque

1. main: validación de argumentos de línea de comandos y conversión del puerto.
2. Se instancia `Server server;`.
3. Se llama `server.loop(port)` (implementación en `Socket/Server`): inicializa socket, `bind`, `listen` y `epoll`.
4. `Server` entra en un bucle principal donde espera eventos de `epoll` para:
   - Nuevas conexiones (accept)
   - Lectura de datos desde clientes (recv)
   - Preparación/permiso para enviar datos a clientes (write-ready)

Salida esperada: sockets listos para aceptar y atender clientes.

## Flujo de conexión de un cliente

1. `accept()` en el socket de escucha devuelve un descriptor `fd_client`.
2. Se crea un `User` (posiblemente con valores por defecto) y se llama `Server::addUser(User u)`:
   - `clients.push_back(u)` — añade objeto `User`.
   - `client_fds.push_back(fd_client)` — guarda el fd paralelo.
   - `messages.push_back(empty queue)` — crea la cola de salida para ese usuario.
3. El nuevo FD se registra en `epoll` para eventos de lectura (y potencialmente escritura).

Puntos de integración para canales:
- Ninguno directo en esta fase; la asociación a canales ocurre luego, cuando el usuario emite `JOIN`.

## Flujo de recepción de datos y parseo

1. Cuando `epoll` indica datos disponibles en `fd_client`, el servidor llama `recv()` y acumula bytes.
2. Los bytes se entregan al sistema de parseo (`Messaging/ParserMessage`) que: 
   - Tokeniza la línea según el protocolo IRC.
   - Genera una representación lógica (por ejemplo, un `Message` con `verb` y `params`).
3. El `Message` se despacha al handler correspondiente (`fnHandlers.cpp`) según el verb (ej. `NICK`, `USER`, `JOIN`, `PRIVMSG`, etc.).

Puntos de integración para canales:
- Aquí se detecta `JOIN #channel`, `PART`, `PRIVMSG #channel` y demás comandos de canal.
- El handler es el lugar principal para invocar la lógica de `Channel`.

## Flujo de un handler de comando (alto nivel)

1. El parser identifica el verb; se llama al handler correcto pasando el `Server`, `User` remitente y el `Message`.
2. El handler valida parámetros y estado del `User` (ej.: usuario identificado).
3. Según el comando:
   - `JOIN #chan`:
     - Buscar canal existente (`Server::get_by_channel_name` o iterar `servers`).
     - Si no existe, crear un nuevo `Channel` y registrarlo con `Server::addChannel`.
     - Llamar `channel.add_member(user)` o equivalente.
     - Preparar mensajes de bienvenida/estado y encolarlos con `Server::add_msg` a todos los miembros.
   - `PART #chan`:
     - Localizar canal, llamar `channel.remove_member(user)`.
     - Si el canal queda vacío, eliminarlo de `Server::servers`.
     - Enviar notificaciones a miembros.
   - `PRIVMSG #chan :mensaje`:
     - Localizar canal, iterar miembros y usar `Server::add_msg` para encolar el mensaje a cada miembro (respetando modos/privilegios).

Puntos de integración para canales:
- Point A (Command entry): en el dispatcher del parser -> aquí se decide qué handler invocar.
- Point B (Handler internals): dentro del handler, se debe interactuar con `Channel` y `Server` para crear/actualizar/leer el estado del canal.

## Flujo de envío de datos (salida)

1. Los handlers que generan salidas no las envían directamente; encolan buffers en `Server::messages` para cada receptor usando `Server::add_msg`.
2. El bucle principal del `Server` vigila `epoll` para eventos de escritura (FD listo para `send`).
3. Cuando `fd` es escribible, el servidor toma mensajes de la cola correspondiente y hace `send()` hasta que la cola esté vacía o el socket bloquee.
4. Si un mensaje estaba marcado como `is_heap == true`, cuando se le da free se hace `delete[]` para evitar fugas.

Puntos de integración para canales:
- Ninguno especial: los mensajes de canal se generan por handlers y salen por las rutas normales de envío.

## Gestión de estado y datos persistentes en runtime

- `Server` mantiene contadores: `max_client_id`, `max_channel_id` (inicializados a 0) que pueden usarse para asignar IDs estables.
- `Server::nick_history` almacena históricos de nicks.
- `servers` contiene `Channel` con su estado en memoria (miembros, topic, modos).

Puntos de integración para canales:
- Al crear un `Channel` nuevo, asignar `id = ++max_channel_id` y almacenar en `servers`.

## Limpieza y desconexión

- `disconnect_user(user_index)` limpia mensajes pendientes del usuario, elimina el FD de `epoll`, cierra el socket y borra entradas en los vectores paralelos.
- Desconexión por parte del servidor o cliente debe notificar a los canales: iterar `servers` y llamar `channel.remove_member(user)` para mantener consistencia.

Puntos de integración para canales:
- Point C (Disconnect hook): cuando se desconecta un usuario, asegurarse de quitarlo de cualquier canal y notificar a los miembros restantes.

## Puntos exactos (resumen) donde entrar los canales

- Punto A — Dispatcher del parser/handler: la primera decisión para comandos de canal (JOIN/PART/PRIVMSG). Implementación: en `Messaging/ParserMessage` y `Messaging/fnHandlers.cpp`.
- Punto B — Dentro del handler: crear o recuperar `Channel` a través de `Server` y llamar a métodos de `Channel` para mutar estado.
- Punto C — En desconexión `Server::disconnect_user`: antes/tras borrar al cliente de `clients`, quitar al usuario de los canales y encolar notificaciones.
- Punto D — En el flujo de inicialización de `Server` o al procesar configuración: opcionalmente, cargar canales persistentes o políticas desde archivos.

## Reglas prácticas y recomendaciones de diseño (sin código)

- Mantener IDs estables: no depender de índices vectoriales directamente, usar `User::get_id()` y `Channel::get_id()`.
- Evitar acoplamientos fuertes: los handlers deben operar con interfaces (p. ej. `Channel` tiene `add_member`, `broadcast`) y delegar a `Server` para encolar mensajes.
- Manejar fallos de I/O: si `send()` o `recv()` fallan, usar un camino consistente para desconectar y limpiar al usuario.
- Limpieza de memoria: decidir una política clara de quién libera buffers (emisor o consumidor) y documentarla.

## Ejemplos de escenarios y flujo esperado (sin código)

- Usuario A hace `JOIN #test`:
  - Parser->Handler `JOIN` -> buscar canal; si no existe crear y `addChannel` -> `add_member(A)` -> encolar `RPL_TOPIC` y `RPL_NAMREPLY` a A.

- Usuario B hace `PRIVMSG #test :hola`:
  - Parser->Handler `PRIVMSG` -> localizar `#test` -> `broadcast` -> encolar el mensaje para A y B según reglas.

- Usuario A desconecta inesperadamente:
  - `Server` detecta EOF/ERROR -> `disconnect_user()` -> remover A de `clients` -> iterar `servers` y `remove_member(A)` -> si canal vacío, borrar canal de `servers`.

## Checklist mínimo para añadir soporte de canales

1. Implementar API completa en `Messaging/Channels/Channel.{hpp,cpp}` (miembros, broadcast, modos, topic).
2. Modificar/añadir handlers en `Messaging/fnHandlers.cpp` que usen `Server` y `Channel`.
3. Actualizar parser si hace falta para normalizar parámetros (ej. nombres de canales con `#`).
4. Añadir notificaciones apropiadas via `Server::add_msg`.
5. Gestionar limpieza en desconexiones y en el destructor de `Server`.

---

Archivo generado: `Documentacion/Flujo_Ejecucion_Detallado.md`

He creado el archivo en `Documentacion/`. ¿Quieres que también añada un diagrama de flujo simple en ASCII o que genere las firmas de función sugeridas (sin implementar) para pegarlas en `Messaging/Channels/Channel.hpp`?  
