
| Categoría                | Comandos mínimos                                             |
| ------------------------ | ------------------------------------------------------------ |
| **Registro**             | `PASS`, `NICK`, `USER`, `PING`, `PONG`                       |
| **Sesión**               | `QUIT`                                                       |
| **Canales**              | `JOIN`, `PART`, `PRIVMSG`, `MODE`, `TOPIC`, `INVITE`, `KICK` |
| **Opcionalmente útiles** | `NOTICE`, `NAMES` (para mejorar compatibilidad con clientes) |


| Comando              | Código                       | Nombre simbólico       | Descripción                       |
| -------------------- | ---------------------------- | ---------------------- | --------------------------------- |
| `NICK`               | `433`                        | `ERR_NICKNAMEINUSE`    | El nick ya está en uso            |
| `USER`               | `461`                        | `ERR_NEEDMOREPARAMS`   | Falta parámetro en el comando     |
| `PASS`               | `464`                        | `ERR_PASSWDMISMATCH`   | Contraseña incorrecta             |
| `PASS`/`NICK`/`USER` | `462`                        | `ERR_ALREADYREGISTRED` | Ya se completó el registro        |
| `LOGIN OK`           | `001`                        | `RPL_WELCOME`          | Bienvenida                        |
|                      | `002`                        | `RPL_YOURHOST`         | Info del servidor                 |
|                      | `003`                        | `RPL_CREATED`          | Fecha de creación                 |
|                      | `004`                        | `RPL_MYINFO`           | Info de versión y modos           |
| `PING`               | `PONG` literal (no numérico) | –                      | Se responde con `PONG <token>`    |
| `JOIN`               | `332`                        | `RPL_TOPIC`            | Tópico del canal (si existe)      |
|                      | `353`                        | `RPL_NAMREPLY`         | Lista de usuarios en canal        |
|                      | `366`                        | `RPL_ENDOFNAMES`       | Fin de la lista de nombres        |
| `PRIVMSG`            | `401`                        | `ERR_NOSUCHNICK`       | No existe nick/canal              |
|                      | `404`                        | `ERR_CANNOTSENDTOCHAN` | No puede enviar (por modos, etc.) |
| `MODE`               | `472`                        | `ERR_UNKNOWNMODE`      | Modo no válido                    |
|                      | `324`                        | `RPL_CHANNELMODEIS`    | Devuelve modos actuales           |
| `TOPIC`              | `331`                        | `RPL_NOTOPIC`          | Canal sin topic                   |
|                      | `332`                        | `RPL_TOPIC`            | Devuelve el topic                 |
| `INVITE`             | `341`                        | `RPL_INVITING`         | Confirmación de invitación        |
| `KICK`               | `482`                        | `ERR_CHANOPRIVSNEEDED` | Falta privilegio de operador      |
|                      | `441`                        | `ERR_USERNOTINCHANNEL` | Usuario no está en canal          |
|                      | `442`                        | `ERR_NOTONCHANNEL`     | Tú no estás en el canal           |
|                      |                              |                        |                                   |

### Niveles de deteccion de errores

Cuando parses un comando, hay dos niveles de detección de errores:

1. **Errores globales del comando**
    
    - Falta de parámetros → `ERR_NEEDMOREPARAMS (461)`
        
    - Usuario no registrado → `ERR_NOTREGISTERED (451)`
        
    - Comando desconocido → `ERR_UNKNOWNCOMMAND (421)`
        
2. **Errores específicos por target**
    
    - Nick inexistente → `ERR_NOSUCHNICK (401)`
        
    - Canal inexistente → `ERR_NOSUCHCHANNEL (403)`
        
    - Nick ya en uso → `ERR_NICKNAMEINUSE (433)`
        
    - No estás en canal → `ERR_NOTONCHANNEL (442)`
        
    - No tienes permisos → `ERR_CHANOPRIVSNEEDED (482)`


### Comma separated list

|Caso|Asociación|
|---|---|
|Varias listas separadas por comas en distintos parámetros|Se asocian **por posición** (1–1, ignorando sobrantes)|
|Una lista comma-separated y los demás parámetros simples|La lista aplica a todos los valores simples|
|Parámetro _trailing_ (`:mensaje`)|No se repite; se aplica a todos los targets|
|Parámetros sin comas|Se usan tal cual|

| Comando         | Parámetros relevantes                            | Tipo de asociación        | Descripción detallada                                                                                                                            |
| --------------- | ------------------------------------------------ | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| **JOIN**        | `<channels> [<keys>]`                            | ✅ **Paralela (1–1)**      | Cada canal puede tener su propia clave. Si hay más canales que claves, las faltantes usan clave vacía.                                           |
| **PART**        | `<channels> [<message>]`                         | ⚙️ **Una lista única**    | El mensaje (si existe) se aplica a todos los canales listados. Ejemplo: `PART #a,#b :bye` → mismo texto para ambos.                              |
| **PRIVMSG**     | `<receivers> :<text>`                            | ⚙️ **Una lista única**    | Se envía el mismo texto a todos los nicks/canales de la lista. Ejemplo: `PRIVMSG alice,bob :hola`.                                               |
| **NOTICE**      | `<receivers> :<text>`                            | ⚙️ **Una lista única**    | Igual que `PRIVMSG`, pero sin respuestas automáticas.                                                                                            |
| **KICK**        | `<channels> <users> [<comment>]`                 | ✅ **Paralela (1–1)**      | Se emparejan por índice: `KICK #a,#b alice,bob` → expulsa `alice` de `#a` y `bob` de `#b`. Si solo hay un canal, se aplica a todos los usuarios. |
| **INVITE**      | `<nick> <channel>`                               | 🚫 **Sin listas**         | Solo un nick y un canal por comando.                                                                                                             |
| **TOPIC**       | `<channel> [<topic>]`                            | 🚫 **Sin listas**         | Solo un canal.                                                                                                                                   |
| **MODE**        | `<target> [<modestring> [<args>...]]`            | ⚙️ **Depende del target** | Si `target` es un canal (`#...`), no se usan listas. Si fuera de usuario (no obligatorio en tu proyecto), tampoco.                               |
| **QUIT**        | `[<message>]`                                    | 🚫 **Sin listas**         | Se aplica solo al propio usuario.                                                                                                                |
| **NICK**        | `<nickname>`                                     | 🚫 **Sin listas**         | Un nick por comando.                                                                                                                             |
| **USER**        | `<username> <hostname> <servername> :<realname>` | 🚫 **Sin listas**         | Un único usuario.                                                                                                                                |
| **PASS**        | `<password>`                                     | 🚫 **Sin listas**         | Solo una contraseña.                                                                                                                             |
| **PING/PONG**   | `<server1> [<server2>]`                          | 🚫 **Sin listas**         | Solo un valor (token o destino).                                                                                                                 |
| **MODE (list)** | `<channel> +o/-o nick1,nick2`                    | ✅ **Paralela (1–1)**      | En teoría se permite listar varios nicks si el modo lo acepta (ej. varios `+o`). No obligatorio en 42.                                           |