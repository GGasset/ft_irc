
# Guía de implementación de un servidor IRC en C++ (sin sockets)

Esta guía presenta una lista exhaustiva de tareas atómicas necesarias para desarrollar la lógica de un servidor IRC en C++ (excluyendo la capa de comunicación por sockets). Se organiza por fases de **Diseño**, **Implementación**, **Pruebas** y **Otros aspectos opcionales**, cubriendo diversas áreas funcionales (estructuras internas, parser, comandos, manejo de respuestas, etc.). Sirve como especificación y hoja de ruta para construir paso a paso un servidor IRC conforme a RFC 2810-2813.

## Fase de Diseño

### Diseño de Estructuras Internas (Usuarios, Canales, Estado Global)

- **Definir estructura `User`**: Crear una clase/struct para representar al usuario/cliente con campos como: `nickname` (apodo único), `username` (nombre de usuario), `hostname` (nombre de host o IP), `serverOrigin` (servidor de origen/local), y `realName` (nombre real si se requiere). Incluir un **historial de nicknames** (lista de apodos previos) para soporte de funcionalidades como WHOWAS. También un indicador de **registro completo** (p.ej. boolean `isRegistered`) para saber si ya realizó NICK+USER.
    
- **Definir estructura `Channel`**: Crear clase/struct para canal con atributos: `name` (nombre del canal), `topic` (tema del canal, con quizás campo para quién lo estableció y cuándo), y **modos de canal** (flags booleanos o bitfield para cada modo: ejemplo `inviteOnly`, `moderated`, `topicOpOnly`, etc.). Los modos corresponden a los definidos por RFC 2811 sección 4: por ejemplo `o` (operador), `v` (voz), `i` (solo invitación), `t` (solo op puede cambiar topic), `k` (clave/password), `l` (límite de usuarios), `m` (moderado), `n` (no externos), `p` (privado), `s` (secreto), `r` (reop), `q` (silencioso), `a` (anónimo), `b` (ban), `e` (excepción), `I` (invita automática)[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=O%20,give%2Ftake%20the%20voice%20privilege)[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=k%20,the%20user%20limit%20to%20channel). Para cada modo, planificar cómo representarlo (por ejemplo, un conjunto de flags y estructuras auxiliares para modos con parámetros).
    
- **Listas de control en `Channel`**: Incluir colecciones para _miembros_ del canal (p. ej. un `std::map<User*, MemberStatus>` o `std::unordered_set` de usuarios en el canal). Cada miembro tendrá indicadores de estado _operador_ (`o`) o _voz_ (`v`). Modelar esto sea con un bitflag en una estructura `MemberStatus` o mediante listas separadas (lista de ops, lista de voces) según convenga. Asimismo, añadir:
    
    - `banList` (lista/conjunto de máscaras baneadas),
        
    - `exceptionList` (lista de máscaras que evitan el ban),
        
    - `inviteList` (lista de usuarios o máscaras invitados al canal).
        
    - Campo `key` (clave de acceso si el canal tiene modo `+k`) y campo `userLimit` (número máximo de usuarios si modo `+l`).
        
- **Estructuras globales del servidor**: Diseñar una clase `ServerState` o similar que mantenga el estado global:
    
    - Un **mapa de usuarios** (por nickname → objeto `User`), para búsqueda rápida de clientes por su apodo. Debe ser case-insensitive (los apodos IRC son case-insensitive[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=Channels%20names%20are%20strings%20,Channel%20names%20are%20case%20insensitive)) – esto se puede lograr normalizando a minúsculas la clave o usando un comparador personalizado.
        
    - Un **mapa de canales** (por nombre de canal → objeto `Channel`), también case-insensitive (recordar que los nombres de canal distinguen `#`, `&`, etc., y no permiten espacios ni ciertas puntuaciones[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=Channels%20names%20are%20strings%20,Channel%20names%20are%20case%20insensitive)).
        
    - Información de configuración general: por ejemplo, nombre del servidor (usado al prefijar respuestas), versión, mensaje MOTD, límites (p. ej. número máx. de canales que un usuario puede unir, longitud máxima de nick, etc.).
        
    - Tabla de **handlers de comandos**: un mapa de string (nombre de comando IRC, e.g. `"JOIN"`, `"PRIVMSG"`) a punteros a función, functors o lambdas que implementen la lógica de cada comando. Esto permite despachar fácilmente mensajes entrantes al manejador correspondiente.
        
- **Relaciones y referencias**: Decidir cómo manejar las referencias entre objetos. Por ejemplo, `Channel` podría guardar punteros crudos o inteligentes (`shared_ptr`/`weak_ptr`) a los `User` miembros. `User` podría tener una colección (lista) de punteros o nombres de canales en los que participa para facilitar iterar sus canales (util para quit). Asegurarse de evitar referencias colgantes: cuando un usuario sale, habrá que removerlo de todos los canales; cuando un canal queda vacío, habrá que destruirlo. Documentar estas invariantes de estado para tenerlas en cuenta en la implementación.
    
- **Unicidad y consistencia**: Definir que los nicknames deben ser únicos en el servidor (no puede haber dos usuarios con el mismo nick). Esto se asegurará usando el mapa global (inserción falla o se verifica antes). Asimismo, garantizar unicidad de nombres de canal (el mapa de canales lo gestiona). Considerar normalizar los nombres/nicks a una forma consistente para la clave (p. ej. mayúsculas). Además, registrar en `User` el `hostname` reportado o detectado, que formará parte del prefijo en los mensajes (formato `<nick>!<username>@<hostname>`).
    
- **Historial de nicknames**: Planear cómo mantener el historial de apodos: por ejemplo, en la estructura `User` incluir un contenedor (lista o cola circular) donde se almacenen apodos antiguos cuando el usuario cambia de nick. Esto servirá para implementar el comando WHOWAS en el futuro. Decidir el tamaño máximo de historial a guardar por usuario (p. ej. últimos 10 nicks).
    

### Diseño del Parser LL(1) de Mensajes IRC

- **Especificación de gramática**: Basarse en la gramática del protocolo IRC (RFC 2812) para estructurar el parser. Un mensaje IRC genérico tiene la forma: `[':' <prefix> <SPACE>] <command> <params> CRLF`[colinjs.com](https://colinjs.com/WinShoeHelp200/WinShoe_Ref.htm#:~:text=RFC2812%20defines%20the%20syntax%20for,params%20%5D%20crlf). Es decir, opcionalmente un **prefijo** (comenzando por `:`) seguido de un comando y, opcionalmente, parámetros separados por espacios, terminado en `CRLF` (retorno de carro + nueva línea). El último parámetro puede contener espacios si está precedido por `:` (denominado _trailing parameter_, que absorbe el resto de la línea)[colinjs.com](https://colinjs.com/WinShoeHelp200/WinShoe_Ref.htm#:~:text=message%20%3D%20%5B%20,params%20%5D%20crlf). Teniendo esto en cuenta, diseñar el parser como un analizador LL(1) predecible (un token de lookahead es suficiente), dado que la sintaxis es relativamente sencilla.
    
- **Separación de flujo en líneas (CRLF)**: Planificar un mecanismo para dividir el flujo de datos TCP en mensajes discretos por el delimitador `\r\n`. Por ejemplo, usar un buffer de entrada incremental: acumular bytes recibidos y buscar secuencias CRLF. Cada vez que se encuentre un CRLF, extraer una línea completa para parsear. Si llega un fragmento sin CRLF final, mantenerlo en el buffer hasta completar. Este componente debe manejar casos donde lleguen múltiples comandos en un solo paquete o uno fragmentado en varios. Ignorar líneas vacías (que son solo CRLF).
    
- **Lexer (analizador léxico)**: Definir cómo tokenizar cada línea una vez separada:
    
    - Representar tokens relevantes: por ejemplo, `SPACE` (una o más espacios separadores), `COLON` (el carácter `:` que indica inicio de prefijo o de último parámetro), `WORD` (secuencia alfanumérica para comandos o parámetros sin espacios), `NUMBER` (comando numérico de 3 dígitos), y quizá token especiales para CRLF (aunque al trabajar línea a línea, CRLF solo delimita el final).
        
    - _Normalización de espacios_: Los parámetros se separan por uno o más espacios. Decidir que el lexer tratará cualquier secuencia de espacios contiguos como un solo separador lógico. También, ignorar espacios iniciales de línea (si llegaran, aunque según protocolo un mensaje no debería empezar con espacio salvo tras prefijo).
        
    - El `prefix` si existe siempre empieza después de un `:` al inicio de la línea. Así que si el primer carácter es `:`, el lexer debe captar todo hasta el siguiente espacio como el token `PREFIX`. Ese prefijo puede contener `!` y `@` (formato nick!user@host) o ser simplemente un nombre de servidor. No es necesario descomponerlo más en el parser (se puede almacenar tal cual, o dividir en subcampos opcionalmente).
        
    - El `command` puede ser una palabra de letras (ej. "JOIN") o un código numérico de tres dígitos (ej. "332"). Para propósitos de sintaxis, cualquier secuencia continua sin espacios hasta encontrar un espacio o fin de línea puede tomarse como el comando. La validación de si son letras o dígitos se hará después.
        
    - Los `params`: Iterativamente, después del comando, cada token separado por espacios es un parámetro. Si un parámetro comienza por `:`, significa que es el _trailing param_ y abarcará el resto de la línea (incluyendo los espacios posteriores como parte del valor). El lexer puede sencillamente marcar ese `:` especial y tomar todo lo que sigue como un solo token paramétrico final, luego no debería haber más tokens tras él.
        
- **Parser LL(1)**: Con los tokens definidos, implementar un parser descendente que analice la estructura:
    
    1. **Prefijo opcional**: Si el primer token es `PREFIX` (detectado por presencia inicial de ':'), asignarlo como `message.prefix` y consumir el siguiente token debe ser un `command`. Si no hay prefijo, continuar directamente con comando.
        
    2. **Comando**: Esperar un token de tipo palabra o número. Si no existe, error de sintaxis (mensaje malformado). Registrar el comando en la estructura del mensaje (posiblemente normalizarlo a mayúsculas para comandos literales).
        
    3. **Parámetros**: Si no hay más tokens tras el comando, está permitido (algunos comandos no tienen parámetros). Si hay tokens, consumirlos secuencialmente como parámetros:
        
        - Usar una lista para almacenarlos.
            
        - Si un token comienza con `:` (y es el primero de los parámetros restantes), tratar todo el contenido _después_ de ese `:` como un parámetro completo (incluyendo espacios dentro, el lexer ya habrá proveído el resto de la línea como uno). Ese parámetro se considera el _trailing param_ y luego no debe haber más parámetros.
            
        - Si un token es un `WORD` normal (no prefijado con `:`), añadirlo como parámetro “middle” (medio). Continuar hasta agotar tokens o hasta haber recopilado el máximo de 15 parámetros. Según RFC, el número de parámetros por mensaje se limita a 15[colinjs.com](https://colinjs.com/WinShoeHelp200/WinShoe_Ref.htm#:~:text=where%20the%20prefix%20indicates%20the,to%20convey%20general%20text%20messages) (14 "middle" + 1 "trailing"). Planificar la validación de este límite: si el mensaje de entrada tiene más de 15 componentes separados, el parser puede o bien truncar extras o marcar error. Mejor detectarlo y marcar un error de sintaxis (para luego enviar un error al cliente).
            
    4. **Fin de línea**: Confirmar que tras procesar tokens se llegó al final de la línea (el CRLF ya fue consumido por el separador). Si quedaron tokens inesperados después del trailing param (lo cual no debería pasar si el lexer y parser están correctos), se podría manejar como error.
        
- **Detección y manejo de errores de sintaxis**: Definir estrategias para los errores que el parser debe identificar:
    
    - _(a) Mensaje malformado (falta CRLF)_: Esto se mitiga gracias al buffer por líneas. En caso de nunca recibir CRLF, el mensaje se considera incompleto y el parser debería esperar más datos. No se procesa hasta recibir la terminación. Si llega un carácter no válido (ej. caracteres de control inaceptables), se puede descartar la línea entera o sanitizar.
        
    - _(b) Comando inválido o incompleto_: Si después de un prefijo (si había) no se encuentra ningún token de comando, o el comando token no consiste en solo letras (A-Z) o dígitos (0-9) según se espere. Por ejemplo, un comando con carácter ilegal (espacio interno o símbolo raro) se puede rechazar. Además, si el comando es sintácticamente correcto pero no reconocido por el servidor (comando desconocido), eso se señalará más adelante al procesar (generando error numérico ERR_UNKNOWNCOMMAND). Desde el punto de vista del parser, se puede simplemente marcar el comando textual; la decisión de si es soportado será lógica (fase de ejecución). No obstante, incluir en la especificación que el sistema debe identificar comandos no soportados para responder con el código de error apropiado.
        
    - _(c) Parámetros incorrectos_: Por sintaxis, esto cubre casos como: un parámetro “middle” que contenga espacios (lo cual significaría que el splitting falló) o exceder el máximo de parámetros. También la ausencia de parámetros obligatorios para ciertos comandos (aunque eso se considera más bien error semántico manejado por el comando, e.g. JOIN sin canal produce ERR_NEEDMOREPARAMS). El parser en sí puede simplemente producir la lista de parámetros; la validación de cada comando comprobará si faltan. Sin embargo, planificar deteción de algunos errores genéricos: por ejemplo, si hay `:` indicando trailing param pero está vacío (ej. línea termina justo después del `:`), se podría considerar como param vacío válido (algunos comandos permiten mensaje vacío).
        
- **Estructura de datos del mensaje parseado**: Decidir si conviene crear una estructura `Message` con campos (`prefix`, `command`, lista `params`) que el parser devuelva. Esto facilita pasar la información a la capa de ejecución. Incluir también quizá un indicador o enumeración del tipo de comando (si se quiere mapear inmediatamente el string del comando a una constante). Otra opción es que el parser llame directamente al handler correspondiente, pero es más limpio separarlo.
    
- **LL(1) y elección predictiva**: Documentar cómo el parser es LL(1): la decisión más notable es al inicio si encuentra `:`, entonces parsea prefijo, si no, parsea comando. Luego, tras comando, siempre puede parsear params hasta agotar. No hay ambigüedades significativas, por lo que un sencillo estado lineal funciona. Esto significa que podremos implementar el parser manualmente sin necesidad de herramientas de parser generator.
    

### Diseño de Manejadores de Comandos IRC

- **Enfoque general**: Por cada comando IRC soportado, definir su comportamiento de acuerdo al protocolo. Los comandos se dividen en categorías:
    
    - _Comandos de sesión/usuario_: `NICK`, `USER`, `QUIT`, (y opcionalmente `PASS`, `PING`, `PONG`, etc.).
        
    - _Comandos de canales_: `JOIN`, `PART`, `MODE`, `TOPIC`, `KICK`, `INVITE`, `NAMES`.
        
    - _Comandos de mensajería_: `PRIVMSG` y `NOTICE` (envío de mensajes a usuarios o canales).
        
    - (Otros comandos como `WHO`, `LIST`, `OPER`, etc., podrían considerarse fuera del alcance mínimo, a menos que se especifiquen luego).
        
- **Especificar precondiciones generales**: Muchos comandos requieren que el usuario esté **registrado** (haya completado NICK+USER). Diseñar una verificación global: si un cliente envía cualquier comando distinto a NICK, USER, PASS, PING antes de registrarse, el servidor debe responder con error `451 ERR_NOTREGISTERED` ("You have not registered")[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=451%20%20%20%20,You%20have%20not%20registered) y no procesarlo. Así se asegura la secuencia de registro correcta.
    
- **Manejador `NICK`**:
    
    - Parámetros: espera 1 parámetro obligatorio (el nuevo apodo). Si no se proporciona, se deberá responder con error `431 ERR_NONICKNAMEGIVEN` ("No nickname given").
        
    - Validaciones de formato: comprobar que el nickname propuesto cumple las reglas de IRC: no puede contener espacios, comodines `* ?` ni comas, ni comenzar por dígito (según RFC, solo letras, números, guiones y ciertos símbolos permitidos)[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Nicknames%20are%20non,the%20following%20restrictions). También considerar limitar la longitud (p. ej. 9 caracteres para compatibilidad clásica, o usar la longitud máxima que el servidor declare). Si el nick contiene caracteres inválidos o es demasiado largo, responder con `432 ERR_ERRONEUSNICKNAME` ("Erroneous nickname").
        
    - Unicidad: verificar si el nickname ya está siendo usado por otro usuario en el servidor. La búsqueda se hace en el mapa global de usuarios (case-insensitive). Si está en uso, rechazar con `433 ERR_NICKNAMEINUSE` ("Nickname is already in use")[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=433%20%20%20%20,Nickname%20is%20already%20in%20use).
        
    - Registrar/actualizar nick:
        
        - Si es la primera vez (usuario no registrado aún pero envía NICK antes que USER), crear una entrada en la tabla de usuarios con ese nick provisionalmente y asociar con el objeto `User` de la conexión. Guardar el nick en el objeto `User`.
            
        - Si el usuario ya está registrado (cambio de nick en vivo), entonces:
            
            - Asegurarse de que el nuevo nick pasa las validaciones anteriores (formato y unicidad).
                
            - Remover la entrada antigua del mapa de usuarios y agregar la nueva (para mantener la tabla consistente).
                
            - Actualizar el campo `nickname` del objeto `User` y agregar el antiguo nick a su historial de nicknames.
                
            - Notificar el cambio a nivel de red: se deberá generar un mensaje `NICK` a todos los clientes que compartan canales con este usuario, indicando el cambio (prefijo viejo nick, comando NICK, parámetro nuevo nick). **No** se envía confirmación numérica al que cambia su nick (el feedback es el propio eco del mensaje).
                
    - Concluir registro: Si tras este NICK, el usuario ya ha enviado también los datos de USER, entonces completar el registro del cliente. Esto implica enviar los mensajes de bienvenida: por ejemplo `001 RPL_WELCOME`, `002 RPL_YOURHOST`, `003 RPL_CREATED`, `004 RPL_MYINFO`, etc., según define el protocolo[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely). El RPL_WELCOME típicamente incluye un saludo con el nick, user y host del cliente[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely). Preparar en la infraestructura de respuestas estos textos (ver sección de respuestas).
        
    - Nota: Si el servidor tiene una lista de nick prohibidos (por ejemplo "anonymous" cuando hay canales anónimos[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=The%20channel%20flag%20%27a%27%20defines,generate%20a%20PART%20message%20instead)), también validar y rechazar en su caso.
        
- **Manejador `USER`**:
    
    - Parámetros: típicamente 4: `<username> <mode> <unused> <realname>`. RFC 2812 define que USER lleva estos cuatro, donde `<mode>` es un número de modo de usuario (ignorado generalmente), `<unused>` a veces se usa para hostname en algunas implementaciones pero por RFC se ignora, y `<realname>` puede contener espacios al estar como último parámetro prefijado con `:`. Si faltan parámetros (menos de 4), responder `461 ERR_NEEDMOREPARAMS` ("Need more parameters").
        
    - Verificar estado: Si el usuario ya completó el registro anteriormente (es decir, ya recibió respuesta de bienvenida), cualquier intento de volver a enviar USER debe ser rechazado con `462 ERR_ALREADYREGISTRED` ("You may not reregister")[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=462%20%20%20%20,You%20may%20not%20reregister). Esto evita re-registro.
        
    - Almacenar datos: Guardar en el objeto `User` los campos: `username` (el valor proporcionado), `realName` (el último parámetro). El `<mode>` puede ignorarse o almacenarse como flags de user modes (invisible, etc.) pero por defecto se puede omitir ya que la mayoría de clientes envían 0.
        
    - Completar registro: Si el cliente ya envió un NICK válido antes, entonces tras procesar USER se tiene todo lo necesario para registrarlo. Marcar `User.isRegistered = true`. Generar los mensajes de bienvenida: enviar `RPL_WELCOME`, `RPL_YOURHOST`, `RPL_CREATED`, `RPL_MYINFO` al cliente[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely), indicando que está conectado. Estos incluyen información como: network welcome, nombre de servidor y versión, fecha de creación del servidor, modos de usuario y canal soportados, etc. También enviar el Message of the Day (`RPL_MOTD` y asociados) si está configurado.
        
    - Si el cliente envió primero USER y luego NICK (orden válido también), la conclusión de registro ocurrirá en el manejador NICK una vez tenga USER. Planificar que ambos manejadores chequen la condición "si tengo ya el otro dato, terminar registro".
        
    - Tras registro, el cliente puede empezar a usar todos los comandos normales.
        
- **Manejador `QUIT`**:
    
    - Parámetros: puede llevar un mensaje de salida (trailing param) o ninguno. El mensaje (ej. "Gone offline") suele usarse para notificar a otros el motivo de desconexión.
        
    - Cierre de sesión: Al recibir QUIT, el servidor considera que el cliente se desconecta. No se envían respuestas numéricas de confirmación; en su lugar, se propaga a los demás usuarios. Pasos a diseñar:
        
        - Remover al usuario de todas las estructuras: iterar por cada canal que este usuario integra y removerlo de la lista de miembros.
            
        - Para cada canal en el que estaba, notificar al resto de miembros con un mensaje `QUIT` con el prefijo del usuario y la razón (si se dio) como trailing. **Ojo**: IRC en realidad propaga QUIT de forma global, no por canal, pero los clientes filtran por canal. Implementación sencilla: enviar a cada usuario compartiendo canal con él una notificación QUIT. (La infraestructura puede optimizar para no enviar duplicado a mismo usuario que comparte múltiples canales).
            
        - Manejar canales vacíos: si tras quitar al usuario un canal queda sin miembros, eliminar ese canal de la tabla global (liberando sus recursos). A menos que se trate de canales especiales que permanezcan (por ejemplo, canales safe `!` persisten unos instantes según RFC, pero podemos omitirlo para simplicidad).
            
        - Liberar al usuario: eliminar su entry en el mapa de usuarios global. Cerrar su socket (en la capa de comunicaciones).
            
    - Logging (opcional): Registrar la desconexión con su nick y mensaje.
        
    - _Nota_: Otros usuarios no reciben confirmación directa de QUIT más allá del mensaje de broadcast. El propio usuario que quita no recibe nada (él mismo solicitó la salida).
        
- **Manejador `JOIN`**:
    
    - Parámetros: normalmente `<channel>{,<channel>} [<key>{,<key>}]`. Es decir, se puede solicitar unirse a múltiples canales en un solo comando, separados por comas, con claves correspondientes separadas por comas. Diseñar soporte al menos para la sintaxis básica de uno a la vez, y opcionalmente múltiples. Si no se proporciona ningún canal, responder `461 ERR_NEEDMOREPARAMS` ("Need more parameters").
        
    - Para cada canal solicitado:
        
        - **Validar nombre de canal**: Debe iniciar con un carácter válido (`#`, `&`, `+` o `!`) y no contener espacios, ASCII 7, coma, ni dos puntos (':')[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=Channels%20names%20are%20strings%20,Channel%20names%20are%20case%20insensitive). Si el nombre no cumple, responder `479 ERR_ILLEGALNAME` (algunos servidores implementan este código) o `403 ERR_NOSUCHCHANNEL` ("No such channel") por no existir. Aquí se puede decidir: si el nombre es sintácticamente inválido, es mejor rechazaro.
            
        - **Límite global de canales**: Comprobar cuántos canales ya ha unido el usuario. Si ya alcanzó el máximo permitido (por ejemplo, 10 canales), entonces rechazar con `405 ERR_TOOMANYCHANNELS` ("You have joined too many channels")[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=405%20%20%20%20,they%20have%20joined%20the%20maximum). Este límite puede definirse en la configuración.
            
        - **Canal ya existe?**: Buscar en el mapa global de canales.
            
            - Si no existe, **crear un nuevo canal**: Instanciar `Channel` con el nombre. Inicializar sus listas de miembros, etc. Añadirlo al mapa global. Marcar al usuario que lo une como miembro. Asignarle automáticamente modo operador (`o`): el primer usuario en un canal nuevo se convierte en operador del mismo. Para safe channels (`!` prefijo), también asignar estado de _creador_ (`O`).
                
            - Si el canal existe, comprobar las **restricciones de entrada**:
                
                - Si el usuario ya es miembro del canal, ignorar el comando (no hacer nada, o quizás responder nada). Por protocolo, un JOIN a un canal ya unido suele no generar error, simplemente no se duplica la presencia.
                    
                - **Baneo**: comprobar la `banList` del canal. Si el hostmask del usuario coincide con alguna entrada de ban _y_ no coincide con ninguna excepción en `exceptionList`, rechazar con `474 ERR_BANNEDFROMCHAN` ("Cannot join channel (+b)") – mensaje típico. El usuario no debe unirse.
                    
                - **Invite-only (+i)**: si el canal tiene modo `i` y el usuario no está en la lista de invitados (`inviteList`), entonces rechazar con `473 ERR_INVITEONLYCHAN` ("Cannot join channel (+i)"). (Si el usuario fue invitado, su mask/nick debería estar en `inviteList`; en tal caso permitir la entrada y _removerlo_ de la lista de invitación después de unirse).
                    
                - **Clave (+k)**: si el canal tiene asignada una contraseña y el usuario no proporcionó la clave correcta en el comando JOIN, rechazar con `475 ERR_BADCHANNELKEY` ("Cannot join channel (+k)"). Si el comando incluía múltiples canales, las claves se corresponden por orden; diseñar lectura de la lista de keys y emparejamiento.
                    
                - **Límite de usuarios (+l)**: si el canal está lleno (número de miembros actual == `userLimit` del canal), rechazar con `471 ERR_CHANNELISFULL` ("Cannot join channel (+l)").
                    
                - **Canal seguro/privado**: no afecta la capacidad de unir, solo la visibilidad; no se necesita verificación especial al entrar.
                    
                - Si ninguna restricción bloquea, proceder a unir. Añadir al usuario a la lista de miembros del canal. Limpiar cualquier entrada de invitación para él (ya consumida).
                    
        - **Actualización de estado tras unirse**:
            
            - Agregar el canal a la lista de canales del usuario (para seguimiento).
                
            - Enviar un mensaje de JOIN a todos los miembros del canal (incluyendo al recién llegado) con el prefijo del usuario: `:<nick>!<user>@<host> JOIN <channel>`. Esto notifica a todos de la entrada.
                
            - Gestionar el **tópico**: Tras la unión, el servidor debe notificar al usuario entrante sobre el estado del canal:
                
                - Si el canal tiene un tópico (`channel.topic` no vacío), enviar `332 RPL_TOPIC <channel> :<topic>` al usuario con el tópico actual.
                    
                - Si no hay tópico, enviar `331 RPL_NOTOPIC <channel> :No topic is set`.
                    
            - Enviar la **lista de usuarios (NAMES)** del canal al nuevo usuario: una o varias líneas `353 RPL_NAMREPLY` listando los nicks en el canal y sus prefijos de status (@ para op, + para voice, etc.), seguida de `366 RPL_ENDOFNAMES <channel> :End of NAMES list`[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Numeric%20Replies%3A). Esto permite al cliente poblar su lista de usuarios del canal. El servidor debe formatear RPL_NAMREPLY como `= <channel> :user1 user2 @opuser ...` por ejemplo (siguiendo la convención).
                
            - Opcional: si implementamos tracking de creación de canal, podríamos enviar también `329 RPL_CREATIONTIME <channel> <timestamp>` indicando el tiempo de creación.
                
        - **Múltiples canales en un JOIN**: Si soportado, repetir el proceso para cada uno. Si alguno falla por error (ban, clave incorrecta, etc.), enviar su correspondiente código de error. Continuar intentando con los demás en la lista.
            
- **Manejador `PART`**:
    
    - Parámetros: `<channel>{,<channel>} [<message>]`. Similar a JOIN, puede listar varios canales a salir. El mensaje es opcional (razón de salida).
        
    - Validar que se especifique al menos un canal; de lo contrario `461 ERR_NEEDMOREPARAMS`.
        
    - Para cada canal indicado:
        
        - Revisar existencia del canal en la tabla global. Si no existe, responder `403 ERR_NOSUCHCHANNEL <channel>` ("No such channel").
            
        - Revisar que el usuario sea miembro de ese canal. Si no lo es, responder `442 ERR_NOTONCHANNEL <channel>` ("You're not on that channel").
            
        - Si sí es miembro:
            
            - Remover al usuario de la lista de miembros del canal.
                
            - Si el usuario tenía privilegios (op/voice), ya no importan al irse. Si era el único operador, no es necesario reasignar porque canal puede quedarse sin op (hasta que alguien más tenga op o canal cierre).
                
            - Opcional: si el usuario había establecido modos específicos, no es necesario limpiarlos manualmente, simplemente queda fuera.
                
            - Notificar a los demás miembros con un mensaje `PART`: prefijo del usuario, comando PART y el canal (y razón si provista: `:<message>`). El que parte también debería ver este PART (los servidores suelen enviar el PART tanto a los demás como al propio cliente que se va, para que su interfaz sepa quitar ese canal).
                
            - Si se proporcionó un mensaje de partida, incluirlo en el mensaje PART. Si no, puede enviarse un mensaje por defecto (algunos servidores usan el nick como mensaje por defecto, pero no es obligatorio).
                
            - Manejar canal vacío: Si tras la partida el canal queda sin usuarios, eliminarlo de la estructura global y liberar recursos asociados. (RFC nota que para canales normales, cuando el último usuario sale el canal desaparece).
                
    - Si se listaron múltiples canales, procesar cada uno y enviar errores o PARTs correspondientes.
        
- **Manejador `MODE`**: Este comando es complejo ya que maneja modos tanto de usuario como de canal. Aquí nos centraremos en modos de canal y los más relevantes:
    
    - Parámetros: para canal, `<channel> {[+|-]modes} [modeparams...]`. Para usuario, `<nickname> {[+|-]modes}` (sin parámetros adicionales ya que los modos de usuario no suelen requerir parámetro).
        
    - **Diferenciar objetivo**: Tras parsear, decidir si el primer parámetro inicia con `#`, `&`, `+` o `!` (indicativo de canal) o de lo contrario es un nickname (modo de usuario):
        
        - **Modo de canal**:
            
            - Validar que el canal existe; si no, `403 ERR_NOSUCHCHANNEL`.
                
            - Si no se especificaron cambios de modo (el comando es solo "MODE #canal" sin más), entonces es una consulta del estado actual. Responder con `324 RPL_CHANNELMODEIS <channel> <modes> [<mode params>]` enumerando los modos activos en el canal y sus parámetros (por ejemplo, `+ntk secretkey 50` si tuviera +n +t +k=secretkey y +l=50)[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,403). Luego enviar `329 RPL_CREATIONTIME <channel> <timestamp>` con la fecha de creación del canal si se desea. No se requiere ser operador para consultar modos, cualquiera puede.
                
            - Si sí se proporcionan cambios (una cadena de modos):
                
                - **Permisos**: Verificar si el usuario solicitante es operador del canal. Si no lo es, la mayoría de cambios no están permitidos. En tal caso, rechazar con `482 ERR_CHANOPRIVSNEEDED <channel> :You're not channel operator`. (Algunas excepciones: un no-op puede bajar modos que afecten solo a sí mismo? En general no).
                    
                - **Parseo de la string de modos**: Recorrer la cadena de modos carácter por carácter:
                    
                    - Interpretar un `'+'` o `'-'` para establecer el _modo de operación_ actual (añadir o quitar). Aplicará a los modos siguientes hasta que cambie. Por ejemplo "+ov-v" significa +o, +v, -v secuencialmente.
                        
                    - Para cada modo letter (ej. 'o', 'p', 'k', etc.), determinar si requiere un parámetro:
                        
                        - Modos **que toman parámetro al agregar**: `o`/`v` requieren un <nick>; `l` (limit) requiere <num>; `k` (key) requiere <clave>; `b` (ban mask), `e` (exception mask), `I` (invite mask) requieren <mask>. Al remover (`-`), algunos siguen requiriendo parámetro: `o`/`v` requieren nick para quitar privilegio, `b`/`e`/`I` requieren la mask específica a quitar. En cambio `-k` no requiere parámetro (quita la clave sin importar cuál era) y `-l` tampoco (desactiva límite).
                            
                        - Modos **que no toman parámetro**: `i, t, n, m, p, s, r, q, a` son flags booleanos que no necesitan parámetro al añadir o quitar. También `O` (creador) y `q` (quiet) no aplican para usuarios comunes (O solo lo asigna el servidor al crear safe channel, q solo manejado por servidores).
                            
                    - Validar que si un modo requiere parámetro, exista el siguiente parámetro en la lista. Si falta, responder `461 ERR_NEEDMOREPARAMS <command>` (aquí <command> sería "MODE") indicando falta.
                        
                    - Aplicar el cambio al estado del canal:
                        
                        - Si es **dar/quitar op (`o`)**: se espera un nick param. Verificar que ese usuario esté en el canal. Si se añade, ponerlo en la lista de operadores; si se quita, removerlo. Actualizar su MemberStatus. Si el usuario que ejecuta no es op o no es a sí mismo, ya habríamos bloqueado arriba; suponer autorizado.
                            
                        - **Dar/quitar voz (`v`)**: similar, ajustar permiso de voz del miembro.
                            
                        - **Invite-only (`i`)**, **moderado (`m`)**, **no externos (`n`)**, **privado (`p`)**, **secreto (`s`)**, **reop (`r`)**, **topic op-only (`t`)**, **anónimo (`a`)**, **quiet (`q`)**: activar o desactivar la bandera booleana correspondiente en el objeto Channel. (Nota: `a` y `q` son especiales; `q` solo server, se podría bloquear si un usuario intenta; `a` se puede permitir solo en ciertos tipos de canal, pero podríamos permitir ops togglearlo en canales &). Si se desconoce un modo letra (no está soportado), rechazar con `472 ERR_UNKNOWNMODE <char> :is unknown mode`.
                            
                        - **Clave (`k`)**: para +k, establecer la clave del canal al valor dado (y marcar flag `hasKey=true`); para -k, remover la clave (borrar el valor, marcar flag como off). Si la clave param está incorrecta (no hay param con +k), error de parámetros.
                            
                        - **Límite (`l`)**: para +l, tomar el número dado y establecer `userLimit` del canal, marcar flag; para -l, desactivar límite (posiblemente ignorar cualquier número param si se dio, o no requerir param). Si se pone +l 0, se podría interpretar como quitar límite también.
                            
                        - **Ban (`b`)**: para +b, añadir la máscara dada a `banList` del canal. Para -b, quitar esa máscara de la lista (si coincide exactamente).
                            
                        - **Exception (`e`)**: +e añadir mask a `exceptionList`, -e quitar.
                            
                        - **Invite mask (`I`)**: +I añadir mask a `inviteList`, -I quitar.
                            
                    - Registrar los cambios aplicados para notificar. Normalmente, el servidor envía una sola respuesta de modo consolidada a todos los miembros del canal indicando los cambios realizados: `:<nick>!user@host MODE <channel> +/-<modes> [params...]`. Agrupar los cambios secuencialmente en un mismo mensaje si vienen en un mismo comando. Por ejemplo, if user sets "+ov user1 user2", se envía `MODE #chan +ov user1 user2`. Si los cambios son distintos tipos, a veces se separan en múltiples mensajes si la cantidad de params excede, pero podemos simplificar enviando en uno siempre que sea el mismo comando.
                        
                    - _Listar bans/excepciones/invites_: Si el usuario pone e.g. "MODE #chan +b" sin parámetro tras +b (o "-b" sin param), muchos servidores interpretan esto como **consulta** de la lista de bans. Diseñar esta posibilidad:
                        
                        - Si detectamos que se pidió listar (por ej. modo string es "+b" y no hay param), responder con la lista completa de bans: por cada entrada en banList enviar `367 RPL_BANLIST <channel> <banmask> [<who> <when>]` si se lleva registro de quién puso el ban y cuándo, seguido de `368 RPL_ENDOFBANLIST <channel> :End of channel ban list`. Similar con `+e` (RPL_EXCEPTLIST 348/349) y `+I` (RPL_INVITELIST 346/347).
                            
                        - Solo los operadores del canal pueden ver estas listas por seguridad. Requerir permisos (lo cual ya se tiene si están intentando +b, presumiblemente).
                            
                - Finalmente, tras aplicar todos los submodos, no enviar respuesta numérica de éxito sino difundir el mensaje `MODE` con los cambios a todos en el canal (incluyendo quien lo realizó). Este es el feedback de que la operación tuvo efecto.
                    
        - **Modo de usuario**:
            
            - Si el objetivo es un nickname (posiblemente el del propio usuario):
                
                - Validar existencia: si ese nickname no corresponde a ningún usuario conectado, `401 ERR_NOSUCHNICK`.
                    
                - Si existe pero **no es el propio cliente** solicitante, entonces normalmente solo IRCops pueden cambiar modos de otro (y solo ciertos modos). En nuestro caso, negar: `502 ERR_USERSDONTMATCH :Cannot change mode for other users` (si no implementamos lógica de IRCop).
                    
                - Si es el propio:
                    
                    - Si no hay modos listados (consulta), responder con `221 RPL_UMODEIS <modes>` enumerando sus modos de usuario actuales (p. ej. "+i" si es invisible, etc.).
                        
                    - Si hay cambio solicitado, procesar cada modo letter: los modos de usuario comunes incluyen `i` (invisible), `w` (receive wallops), `o` (IRC operator), `r` (mark as registered user, set by services), etc.
                        
                    - Restringir: el usuario no puede darse a sí mismo +o (operador IRC) mediante MODE, eso solo se obtiene con OPER. Si intenta, responder con `481 ERR_NOPRIVILEGES` o `501 ERR_UMODEUNKNOWNFLAG` ("Unknown MODE flag") para indicarle que no puede. Similar con otras flags desconocidas.
                        
                    - Aplicar los que sí: por ejemplo, si pide "+i", marcar flag de invisible en su objeto User; "-i" quitarlo.
                        
                    - Anunciar el cambio: se puede optar por no enviar ningún mensaje a otros porque los modos de usuario no se propagan a otros clientes (solo server knows, except maybe in WHO replies). Sin embargo, algunos servidores envían al propio usuario un mensaje MODE confirmando el cambio: `:<nick>!user@host MODE <nick> +i`. Podemos implementar que el servidor envíe al usuario un echo de su comando de modo aplicado.
                        
            - Nota: Modo de usuario es opcional; podríamos posponer su implementación ya que no fue explicitamente solicitado. Mínimo soportar la consulta RPL_UMODEIS y quizás invisible mode.
                
- **Manejador `TOPIC`**:
    
    - Parámetros: `<channel> [<topic>]`. Si se proporciona `<topic>` (puede ser cadena vacía ""), significa establecer el tópico; si no, significa consultar el tópico actual.
        
    - Validaciones:
        
        - Verificar que el nombre de canal se dio, si no `461 ERR_NEEDMOREPARAMS`.
            
        - Verificar que el canal existe; si no, `403 ERR_NOSUCHCHANNEL`.
            
        - Si el usuario no es miembro del canal:
            
            - Para _consulta_ de tópico, algunos servidores permiten ver el tópico si el canal es público o invitado (no secreto). En cambio, si el canal es `+s` (secreto) o `+p` (privado) y el solicitante no está dentro, podría responder como si el canal no existe (para secreto) o simplemente no listar tópico. Para simplicidad:
                
                - Si canal es secreto/privado y usuario no dentro, responder `442 ERR_NOTONCHANNEL` (aunque el texto es "You're not on that channel" connotando que no puede ver la info). Esto disuade consultas externas.
                    
            - Para _cambio_ de tópico, si no estás en el canal, obviamente no puedes cambiarlo. Devolver `442 ERR_NOTONCHANNEL`.
                
        - Si el usuario **desea establecer el tópico**:
            
            - Checar modo del canal: si el canal tiene `+t` (solo operadores pueden cambiar tópico) y el usuario **no** es operador, rechazar con `482 ERR_CHANOPRIVSNEEDED <channel> :You're not channel operator`.
                
            - Si permitido: actualizar el `channel.topic` al texto dado. Si el tópico es una cadena vacía (`TOPIC #chan :` con nada), se interpreta como **borrar** el tópico. Entonces establecer topic a vacío y remover el flag de tener tópico.
                
            - Almacenar también quién y cuándo lo cambió, si queremos (muchos servidores guardan `topicSetter` y `topicTime` para respondern en RPL_TOPICWHOTIME). Opcional.
                
            - Difundir a todos los miembros del canal un mensaje `TOPIC`: prefijo del usuario que lo cambió, comando TOPIC, canal y `:<nuevo tópico>`. Esto actualiza el tópico visible en los clientes.
                
            - No enviar RPL_TOPIC numérico como confirmación; la notificación es el broadcast. El que lo cambió verá su propio TOPIC mensaje también.
                
        - Si el usuario **consulta el tópico** (no dio parámetro o param vacío):
            
            - Si el canal tiene un tópico establecido (no vacío): enviar `332 RPL_TOPIC <channel> :<topic>`.
                
            - Si no hay tópico: enviar `331 RPL_NOTOPIC <channel> :No topic is set`.
                
            - (Opcional: si llevamos info de quién/cuándo, mandar `333 RPL_TOPICWHOTIME <channel> <nick> <timestamp>`).
                
- **Manejador `KICK`**:
    
    - Parámetros: `<channel> <user> [<comment>]`. Se puede implementar múltiples usuarios y canales separados por comas también, pero para empezar uno a uno.
        
    - Validaciones:
        
        - Verificar parámetros suficientes: necesita al menos canal y nick objetivo. Si falta cualquiera, `461 ERR_NEEDMOREPARAMS`.
            
        - Checar existencia del canal. Si no existe, `403 ERR_NOSUCHCHANNEL`.
            
        - Checar que el solicitante está en el canal. Si no, `442 ERR_NOTONCHANNEL`. Solo miembros pueden kickear, lógicamente.
            
        - Checar privilegios: si el solicitante no es operador del canal, rechazar con `482 ERR_CHANOPRIVSNEEDED <channel>`. (Solo ops pueden expulsar).
            
        - Encontrar al usuario objetivo en la lista de miembros del canal. Si no está, responder `441 ERR_USERNOTINCHANNEL <user> <channel> :They aren't on that channel`.
            
    - Ejecución:
        
        - Remover al usuario objetivo de la lista de miembros del canal.
            
        - Si el usuario objetivo estaba en el canal, armar un mensaje `KICK <channel> <user>` con prefijo del kicker y con comentario si se dio (por defecto, muchos servidores ponen el nick del kicker o un mensaje genérico si no hay razón; aquí podríamos poner nada o el nick).
            
        - Enviar este mensaje a **todos los miembros restantes** del canal **y** al usuario expulsado. Todos deben ser notificados:
            
            - Los que quedan ven que <kicker> expulsó a <user> (posiblemente con razón).
                
            - El expulsado también recibe el KICK para saber que fue sacado (y su cliente generalmente lo tratará como si hubiera parted).
                
        - Actualizar estado: si el usuario expulsado es local, quitarlo de su lista de canales. Si era el último en el canal (además del kicker), el kicker quedará solo; si luego sale, canal se eliminará como de costumbre.
            
        - No hay respuesta numérica de éxito; el KICK mensaje es la notificación.
            
        - Si se listan múltiples objetivos/canales en un comando (ej. "KICK #chan1,#chan2 user1,user2"), planificar manejar cada par. Para cada, aplicar las reglas anteriores. Errores pueden enviarse si alguno falla.
            
- **Manejador `INVITE`**:
    
    - Parámetros: `<nickname> <channel>`. Invita al usuario dado al canal.
        
    - Validaciones:
        
        - Comprobar que se dieron ambos params, sino `461 ERR_NEEDMOREPARAMS`.
            
        - Verificar si el canal existe:
            
            - Si no existe, responder `403 ERR_NOSUCHCHANNEL`. (Algunos servidores permiten invitar a un canal inexistente para que cuando se cree notifique, pero no es estándar; lo ignoramos).
                
        - Verificar que el solicitante está en el canal:
            
            - Si no está en el canal, `442 ERR_NOTONCHANNEL`. Solo miembros pueden invitar a otros.
                
        - Verificar modo invite-only:
            
            - Si el canal tiene modo `+i` (invite-only) **y** el invitador no es operador, rechazar con `482 ERR_CHANOPRIVSNEEDED`. En canales sin +i, cualquier miembro puede invitar por defecto (esto puede variar en implementaciones, pero usamos esta regla).
                
        - Verificar que el usuario objetivo existe en el servidor:
            
            - Buscar el nick en la tabla global de usuarios. Si no se encuentra, `401 ERR_NOSUCHNICK`.
                
        - Verificar si el usuario objetivo _ya está_ en el canal:
            
            - Si sí está, responder `443 ERR_USERONCHANNEL <user> <channel> :is already on channel` (no tiene sentido invitarlo).
                
    - Ejecución:
        
        - **Añadir a lista de invitados**: Registrar la invitación en la estructura del canal. Puede ser agregando el nick objetivo a `inviteList` del canal, o almacenando una marca en el objeto del usuario invitado. Elegimos mantener en `Channel.inviteList` la lista de nicks invitados que pueden saltarse +i una vez. (También podríamos no usar lista y simplemente permitir la siguiente JOIN de ese nick, pero es más claro guardarlo con timestamp quizá).
            
        - **Confirmación al invitador**: Enviar `341 RPL_INVITING <channel> <nick>` al usuario que realizó la invitación para informarle que la invitación se ha enviado[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Numeric%20Replies%3A).
            
        - **Enviar INVITE al invitado**: Mandar un mensaje directo al usuario objetivo notificándole la invitación: `:<inviter>!user@host INVITE <target> :<channel>`. El invitado recibirá este mensaje como aviso (el cliente típicamente lo muestra). Este mensaje debe ser enviado incluso si el invitado está en otro servidor (en redes distribuidas se pasaría a ese servidor; en nuestro caso monoserver, directamente al usuario local).
            
        - No se requiere enviar nada a los miembros del canal (invitar es silencioso para los demás).
            
        - Si el invitado acepta y hace JOIN posteriormente, el servidor deberá reconocer que estaba invitado (por la lista) y dejarle entrar pese a +i.
            
        - (Opcional: podríamos auto-quitar la invitación después de un tiempo si no se usa, o tras un uso único).
            
- **Manejador `NAMES`**:
    
    - Parámetros: `[<channel>{,<channel>}]`. Sin parámetros significa listar usuarios de _todos_ los canales visibles y usuarios fuera de canales. Con parámetros, listar usuarios de esos canales específicos.
        
    - Comportamiento:
        
        - Si no hay parámetro, para cada canal en el servidor, decidir si se lista:
            
            - Omitir los canales que son `+s` (secretos) en los que el usuario no esté, y quizás mostrar de canales `+p` solo si está en ellos (ya que privados ocultan nombre en /list pero en /names? Según RFC ambos ocultan existencia[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=The%20channel%20flag%20%27p%27%20is,the%20channel%20from%20other%20users), así que probablemente igual). Para simplificar: no listar canales donde no esté si son +p o +s.
                
            - Listar todos los canales públicos o aquellos privados/secretos donde el solicitante es miembro.
                
            - Formato de salida: Para cada canal seleccionado, enviar `353 RPL_NAMREPLY = <channel> :<names>` con los nombres de los usuarios en ese canal. Anteponer `@` a operadores y `+` a voiced. Si hay muchos usuarios, podría requerir múltiples líneas; implementar división si excede 512 bytes la línea.
                
            - Después de todos, enviar un solo `366 RPL_ENDOFNAMES * :End of NAMES list` (cuando no hay parámetro, algunos servidores usan `*` como canal).
                
            - Además, podría enumerar usuarios _no en ningún canal_ bajo un pseudo-canal `*` (muchos IRCds listan "usuarios invisibles" o sin canal). Esto es avanzado; se puede omitir o implementar mostrando those users as in channel "*".
                
        - Si se especifican canales:
            
            - Para cada nombre de canal dado separado por comas:
                
                - Si el canal no existe: simplemente enviar `366 RPL_ENDOFNAMES <channel> :End of NAMES list` inmediatamente para ese nombre, sin ningún 353, (indicando vacío o que no hay tal canal).
                    
                - Si existe:
                    
                    - Si el canal es secreto/privado y el usuario no es miembro, **no** revelar sus usuarios. Se podría enviar un 366 de fin sin listar nada para ese canal (manteniendo su ocultamiento), o posiblemente un 353 con "=" y ningún nombre, pero lo correcto es no exponer su existencia. Para indicar nada, enviar igualmente 366, para cumplir la respuesta terminadora.
                        
                    - Si es visible (público o usuario es miembro), recolectar la lista de nicks con sus prefixes.
                        
                    - Enviar `353 RPL_NAMREPLY <sigil> <channel> :<names>` donde `<sigil>` es "=" para canales públicos, "@" para secretos, "*" para privados (según RFC 1459). Podemos simplificar usando "=" siempre para no complicar.
                        
                    - Enviar `366 RPL_ENDOFNAMES <channel> :End of NAMES list` después de cada canal o después de todas? Tradicionalmente, 366 se envía per canal query. Implementar per canal para claridad.
                        
        - Este comando no requiere ser en el canal para ejecutarlo (excepto restricciones de secreto).
            
        - No produce cambios de estado, solo consultas.
            
- **Manejador `PRIVMSG` / `NOTICE`**:
    
    - Parámetros: `<target>{,<target>} :<text>`. `<target>` puede ser un nick de usuario, un nombre de canal (comenzando por #, etc.), o incluso un mask (servermask for server notices, no implementaremos). Permitir múltiples destinatarios separados por comas en un solo mensaje. `<text>` es el mensaje a enviar (trailing param, puede contener cualquier texto hasta 512b total línea).
        
    - Validaciones comunes:
        
        - Si falta target, responder `411 ERR_NORECIPIENT :No recipient given (<command>)`[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=411%20%20%20%20,%3AWildcard%20in%20toplevel%20domain). Si falta texto (nada tras colon), responder `412 ERR_NOTEXTTOSEND :No text to send`[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=411%20%20%20%20,%3AWildcard%20in%20toplevel%20domain).
            
        - Este comando puede enviarse solo después del registro completo; si llega antes, `451 ERR_NOTREGISTERED`.
            
    - Enviar a **usuario (privado)**:
        
        - Buscar el usuario por nick. Si no existe, responder `401 ERR_NOSUCHNICK <nick> :No such nick/channel`. (El código 401 se usa tanto para nick inexistente como canal inexistente en RFC).
            
        - Si existe:
            
            - Si el comando es PRIVMSG: entregar el mensaje. Formatear un mensaje `:<senderNick>!user@host PRIVMSG <targetNick> :<text>` y enviarlo únicamente al destinatario.
                
            - Si el comando es NOTICE: igualmente formatear `NOTICE` y enviar al dest, _pero_ **no generar ningún error ni respuesta** si el destino no existe o está ausente (ver más abajo la diferencia).
                
            - Manejar estado "away": Si el usuario destino está marcado como ausente (AWAY), y esto es un PRIVMSG, se debería enviar al remitente un `301 RPL_AWAY <nick> :<awaymsg>` indicando que el destino está ausente. Para NOTICE, no se debe enviar respuesta automática de away (ni de ningún tipo)[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,402). (Implementar AWAY es opcional, pero dejar el hook por si acaso).
                
        - Si el destinatario es uno mismo, podríamos simplemente enviarle su propio mensaje o ignorarlo. (Los clientes generalmente impiden esto, pero no está de más decidir qué hacer; podríamos simplemente entregarlo a sí mismo).
            
    - Enviar a **canal**:
        
        - Verificar que el canal existe en la tabla global. Si no, `401 ERR_NOSUCHNICK/ERR_NOSUCHCHANNEL`.
            
        - Si existe:
            
            - Comprobar permisos:
                
                - Si el canal tiene modo `+n` (no permitir mensajes externos) y el remitente **no** es miembro del canal, entonces rechazar el PRIVMSG con `404 ERR_CANNOTSENDTOCHAN <channel> :Cannot send to channel`[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=404%20%20%20%20,%3ACannot%20send%20to%20channel). (Para NOTICE, en lugar de error se **silencia**: simplemente no entregar y no informar al remitente, porque no se responden errores a NOTICE[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,402)).
                    
                - Si el canal tiene modo `+m` (moderado) y el remitente no es miembro con voz u operador:
                    
                    - Si es PRIVMSG: rechazar con `404 ERR_CANNOTSENDTOCHAN` igualmente[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=404%20%20%20%20,%3ACannot%20send%20to%20channel), ya que no puede hablar.
                        
                    - Si es NOTICE: de nuevo, descartar silenciosamente sin error (para evitar loops bots, etc.).
                        
                - Si el remitente está baneado en el canal:
                    
                    - Técnicamente, si está baneado pero aún conectado (¿cómo? podrían estar en canal y luego ser baneado pero no expulsado), o si está fuera intentando enviar externo y baneado, se podría también bloquear. Simplificaremos a: si no es miembro y su host coincide ban (lo que en general le impediría unirse pero quizá no enviar), o si es miembro pero baneado (no usual, bans suelen usarse para no entrar, no para silenciar; algunos IRCd usan +b también para mute junto a +m). Podríamos ignorar esta sutileza o aplicarla como medida extra: no entregar mensaje si su máscara está baneada.
                        
                - De lo contrario, permitido.
                    
            - Entrega del mensaje: para cada miembro del canal **excepto el remitente** (no se suele enviar el mensaje de vuelta al que lo envió, los clientes localmente muestran lo que envían), enviar `:<senderNick>!user@host PRIVMSG <channel> :<texto>`. Esto implica iterar sobre la lista de miembros del canal y escribir a cada socket destino.
                
            - Asegurarse de no enviar a usuarios que se hayan desconectado recientemente (pero esos ya no estarían en la lista).
                
            - Si el canal es moderado +m y el usuario tiene voz o es op, se permite y los demás lo reciben.
                
            - Notar: para NOTICE al canal, se hace lo mismo pero con NOTICE en lugar de PRIVMSG y sin posibilidad de errores hacia el remitente si canal no existe o si estaba bloqueado (simplemente no se entrega y ya).
                
        - Caso especial: si un canal es secreto o privado y el remitente no es miembro, es probable que ni siquiera debería saber de su existencia. Ya manejamos que si +n, no puede enviar; si canal es +p o +s pero no +n (lo cual sería raro, la mayoría de secretos ponen +n también), podríamos permitir el envío si no está +n. Pero por consistencia de privacidad, quizás incluso si no tiene +n, si es secreto y no miembro, podríamos fingir no existe (401). Para no complicar, la regla +n cubrirá la mayoría de casos (los canales suelen tener +n por defecto).
            
    - **Múltiples destinos**:
        
        - Si se listan varios objetivos separados por comas, el servidor debe procesarlos individualmente. Para PRIVMSG:
            
            - Enviar el mensaje a cada destino válido.
                
            - Si alguno falla (por ej. un nick no existe), se puede enviar error 401 para ese objetivo. RFC 1459 define `407 ERR_TOOMANYTARGETS` cuando un mensaje tiene varios destinatarios y alguno falla o hay duplicados, pero eso es más para notificar de múltiples fallos con un solo error. Se podría simplificar: enviar un 401 por cada nick/canal inválido.
                
            - No hay gran riesgo de loops con múltiples objetivos, pero se debe evitar enviar duplicado a la misma persona en caso de repetirse en la lista (no es necesario, pero podríamos filtrar duplicados).
                
        - Para NOTICE con múltiples: No retornar errores (no 401 ni 407) si alguno no existe, simplemente no entregar a esos. Esto sigue la filosofía de no respuestas automáticas a NOTICE[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,402).
            
    - **Ausencia de respuestas**: Aparte de los errores mencionados para PRIVMSG, este comando no tiene respuestas numéricas “de éxito”. La entrega misma es la acción. Para NOTICE, reiteramos: **no** enviar errores o respuestas automáticas (ni siquiera away messages) al remitente[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,402). Esto debe diseñarse cuidadosamente para que el código reutilizado distinga entre PRIVMSG y NOTICE en cuanto a feedback.
        

_(Otros comandos no listados explícitamente como WHO, LIST, PING/PONG, OPER, etc., podrían ser diseñados en caso necesario. Por ejemplo, `PING` debe responder con `PONG` inmediatamente con los mismos datos; `PONG` suele no requerir respuesta. `LIST` enumeraría canales. Estos quedan fuera del alcance principal a menos que se requieran luego.)_

### Diseño de Infraestructura de Respuestas IRC

- **Tabla de códigos de respuesta**: Elaborar una asociación clara entre las acciones/comandos y los posibles mensajes de respuesta (numéricos o no) que el servidor debe enviar. Esto implica listar para cada comando los _replies_ de éxito y los _errors_. Por ejemplo:
    
    - Tras registro (NICK+USER): enviar `001 RPL_WELCOME`, `002 RPL_YOURHOST`, `003 RPL_CREATED`, `004 RPL_MYINFO`[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely), etc., en orden.
        
    - `NICK`: errores posibles `431, 432, 433` (sin respuesta de éxito explícita, salvo el eco de NICK a otros usuarios).
        
    - `USER`: error `461, 462` y en éxito no hay numérico propio, sino que desencadena bienvenidas.
        
    - `JOIN`: errores `461, 473, 475, 471, 474, 405` etc., y en éxito envía topic/NAMES (332/331, 353, 366).
        
    - `PART`: errores `461, 403, 442`.
        
    - `MODE`: errores `461, 482, 472, 401 (if target nick not found), 502`, replies `324, 329` (channel modes, creation time), `221` (user modes), list replies `367/368, 348/349, 346/347` si se implementan.
        
    - `TOPIC`: errores `461, 482, 442, 403`, replies `332/331` para consultas.
        
    - `KICK`: errores `461, 442, 482, 441, 403` (sin reply de éxito).
        
    - `INVITE`: errores `461, 442, 482, 401, 443, 403`, reply `341` en éxito y un INVITE message.
        
    - `NAMES`: no errores a menos canal no existe (pero eso se maneja simplemente con EndOfNames vacío), replies `353, 366`.
        
    - `PRIVMSG`: errores `411, 412, 401, 404` (403 no se usa para privmsg, se usa 401), plus `301` away possibly.
        
    - `NOTICE`: errores: **ninguno enviado** intencionalmente[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=,402), no even 401/404.
        
    - `PING`: error `409 ERR_NOORIGIN` si falta token. `PONG` de respuesta.
        
    - etc.
        
- **Estructura de datos para códigos**: Plantear definir todos estos códigos numéricos y sus mensajes estáticos en una estructura, por ejemplo un `std::map<int, std::string>` de código a texto base (donde <nick>, <channel>, etc., serán insertados dinámicamente). Alternativamente, usar enumeraciones para códigos y tablas de formato. Como los códigos IRC son bien conocidos, se pueden codificar como constantes (p. ej. `const int ERR_NOSUCHNICK=401;`) y tener funciones para producir la string.
    
- **Formato de los mensajes de respuesta**: Todas las respuestas enviadas por el servidor deben respetar la sintaxis IRC:
    
    - Mensajes **numéricos**: Se envían con el formato `:<servername> <code> <target> <params>\r\n`. El `<servername>` es el prefijo indicando quién envía (nuestro servidor). `<code>` es el número de tres dígitos. `<target>` normalmente es el nick del usuario destinatario (especialmente para replies; así el cliente sabe que van dirigidos a él). Luego los parámetros específicos del mensaje. Ejemplo: `:irc.example.com 401 Alice Bob :No such nick/channel`. Aquí "Alice" es el destinatario, "Bob" estaba en los params del comando original (nick buscado), y el texto es la descripción.
        
    - Mensajes **de comandos** (no numéricos): Formato `:<prefix> COMMAND params\r\n`. El `<prefix>` puede ser un usuario (`nick!user@host`) o servidor, según corresponda, representando la fuente del mensaje. E.g., `:Alice!alice@host JOIN #canal` o `:server.name.com NOTICE Auth :*** Looking up your host...`.
        
    - Planificar funciones auxiliares, p.ej. `sendNumeric(User* user, int code, std::initializer_list<std::string> params)`: construye el mensaje con el prefijo del servidor y agrega los parámetros dados (incluyendo quizás el nick destino como primero) y el texto con `:` para el último parámetro si contiene espacios. Otra para `sendMessage(prefix, command, params)` para mensajes normales.
        
    - Asegurarse de siempre terminar con `\r\n`.
        
- **Construcción de mensajes**: Decidir si formatear manualmente con `sprintf`/`std::ostringstream` o usar algo como fmt. Debe insertarse el **target** correcto: Por convención, en replies numéricos el primer parámetro después del código suele ser el nick del destinatario. Por ejemplo, `353 RPL_NAMREPLY` es enviado como `:<server> 353 <nick> = <channel> :user1 user2...`. Incluir ese <nick> automáticamente. El diseño de la tabla de respuestas debe reflejar esto (saber cuándo incluir el destinatario).
    
    - Podríamos codificar plantillas donde se use `%s` para ciertos placeholders. O más simple: construir con concatenaciones ad-hoc en cada caso. Pero una tabla de asociación comando→códigos ayuda a no olvidar alguno.
        
- **Ejemplos de formateo**:
    
    - _Error example_: Al procesar un comando desconocido "FOO", el servidor llamaría a algo como `sendNumeric(user, 421, {cmdName, ":Unknown command"})` que enviaría `:server 421 <usernick> FOO :Unknown command`.
        
    - _Normal reply example_: Después de `USER`, enviar `001` con `:<server> 001 <nick> :Welcome to ... <nick>!<user>@<host>`[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely). Ese texto "Welcome to the Internet Relay Network ..." podemos almacenarlo y formatear con los datos del usuario.
        
- **Sincronización con handlers**: En cada manejador de comando, planificar en qué casos llamar a la función de respuesta. E.g., en JOIN, para errores de key/banned/etc se llama inmediatamente durante validaciones; para éxito, después de actualizar estado, se llama a rutinas que envían topic y names (que internamente usan sendNumeric para 332/331, 353, 366).
    
- **No duplicar lógica de formato**: Idealmente, separar la lógica de _qué_ responder (decidido en handler) de _cómo_ formatear. Los handlers deciden el código y contenidos; la capa de respuestas ensambla la línea.
    
- **Longitud máxima**: Recordar que IRC limita cada mensaje a 512 bytes incluido CRLF[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Message%20Format). En el diseño, prever que funciones de envío corten adecuadamente (por ejemplo, en NAMES, si la lista de usuarios es larga, fragmentarla en varios 353). Mantener contadores de longitud cuando se agregan nombres, etc.
    
- **Codificación**: Asumir uso de UTF-8 para texto, que es el estándar moderno[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Software%20SHOULD%20use%20the%20UTF,Character%20Encodings%20implementation%20considerations%20appendix). No es necesario conversión si internamente manejamos strings Unicode (C++ char* asumimos UTF-8).
    
- **Respuestas automáticas a NOTICE**: Como se señaló, debemos suprimirlas. Esto significa en el handler de NOTICE, no llamar a sendNumeric para errores 401, 404, etc. Podríamos incorporar una bandera en sendNumeric que indique "suppress if notice context" o, más claro, simplemente en el código del handler NO llamar a esos sendNumeric en caso de Notice. Documentar esta decisión.
    
- **Mensajes de sistema**: Decidir cómo enviar notificaciones internas al cliente si se requieren (por ejemplo, un servidor suele enviar un NOTICE del servidor con ciertos avisos al conectar). Estos se formatearían como `:server.name NOTICE <nick> :text`. Tener utilidad para eso.
    
- **Confirmación de comandos**: La mayoría de comandos IRC no tienen "ACK" explícito (lo que pasa es visible a través de otras acciones: e.g. JOIN exitoso se ve por el mensaje JOIN y la lista de usuarios, etc.). Asegurarse de seguir ese patrón para no enviar confirmaciones redundantes.
    
- **Plantilla de errores**: Preparar los textos de error en inglés tal cual RFC (podemos almacenarlos). Ejemplo: ERR_NOSUCHNICK = "%s :No such nick/channel", donde %s será el nick objetivo. Decidir si queremos traducirlos al español – no, el estándar es en inglés. Debemos mantenerlos en inglés para compatibilidad con clientes.
    
- **Mantenimiento**: Documentar claramente en el código qué numeric corresponde a qué situación para facilitar futuras extensiones.
    
- **Ejemplo global**: Documentemos de forma integrada el flujo de una acción como JOIN para verificar: Usuario envía `JOIN #hola`. Servidor maneja, añade a canal, responde con:
    
    1. `:Usuario!user@host JOIN #hola` (a todos en #hola).
        
    2. `:server.name 332 Usuario #hola :<topic>` o 331 si no hay topic.
        
    3. `:server.name 353 Usuario = #hola :@Usuario` (lista de usuarios, con @ porque él es op).
        
    4. `:server.name 366 Usuario #hola :End of NAMES list`.  
        Esto cumple el protocolo.
        

## Fase de Implementación

### Implementación de Estructuras Internas

- **Implementar clase `User`** en C++: con miembros privados para nickname, username, hostname, serverOrigin, realName, isRegistered, etc., y contenedores para channels y nicknameHistory. Proveer métodos públicos para manipularlos:
    
    - `setNickname(const std::string& newNick)`: valida formato (quizá reutilizando una función estática), actualiza nicknameHistory, etc.
        
    - `joinChannel(Channel* channel)`, `leaveChannel(Channel* channel)`: actualiza la lista de canales del usuario.
        
    - Getters y setters simples para username, realName, etc.
        
    - Un campo para modes de usuario (flags bitfield for +i, +o, etc.), si se implementan.
        
    - Constructor que inicializa con socket info (hostname) y perhaps a default generated username if needed.
        
    - Destructor/cleanup: quizá no haga mucho excepto logging.
        
- **Implementar clase `Channel`**: con miembros para nombre, topic, key, userLimit, booleans for each mode flag (or a single bitset for modes to toggle easily by index). También:
    
    - Estructuras de miembros: por ejemplo, `std::unordered_map<User*, uint8_t> members;` donde el uint8_t bitmask indicates op/voice. O mantener `std::unordered_set<User*> memberList` plus separate `opSet` and `voiceSet`. Ambas aproximaciones son válidas:
        
        - Con un map bitmask, un miembro puede tener ambas flags (op+voice, aunque en IRC no tiene mucho sentido tener voice si ya es op, pero técnicamente posible).
            
        - Es fácil chequear membership and status in one structure. Elegir e implementar.
            
    - Estructuras de listas de máscara: `std::vector<std::string> banList, exceptionList, inviteList`. Could also be set for quick lookup, but order not important. Using vector is fine given typically small lists.
        
    - Constructor que inicializa el nombre y quizá fija default modes (por ejemplo, por defecto un canal podría iniciar con +nt en muchas networks - no permitir externos y topic lock, pero eso es política de red. Podemos decidir que por defecto no tiene modos salvo los implicados por tipo de canal).
        
    - Métodos:
        
        - `addMember(User* user, bool asOp=false)`: Añade el usuario a members, setea su op flag según el segundo parámetro. También manejar el caso de primer miembro: quizás siempre asOp para primer user. Retornar éxito/fallo.
            
        - `removeMember(User* user)`: Quita de members. No olvidar también quitar de op/voice sets si esas se usan separadas.
            
        - `hasMember(User* user)`: Ver si user en members.
            
        - `isOperator(User* u)`, `isVoice(User* u)`: chequea en la estructura.
            
        - `setMode(char mode, bool value, [param])`: Cambia la flag booleana correspondiente o lista correspondiente. Ejemplo: para 'k' (clave), si value==true param es la nueva clave (guardar en key, set hasKey true), si value==false, borrar key. Para 'l', similar con userLimit. Para 'b', value true añade param a banList, false quita.
            
        - `getModeString()`: compone una cadena como "+ntk" etc representando modos activos, y retorna también los parametros actuales (key, limit, etc.) para usarlos en RPL_CHANNELMODEIS.
            
        - `getMemberNames()` para NAMES list: itera por members y construye string "[@|+]nick ..." con prefix. Podría ordenarlos alfabéticamente por estética (no requerido).
            
        - `isEmpty()`: returns true si members vacía.
            
        - `invite(User* u)` y `uninvite(User* u)`: agrega/quita de inviteList (usando su nick o user pointer).
            
        - `ban(mask)`, `unban(mask)`: similar for banList.
            
    - Manejar la persistencia de algunas cosas:
        
        - `creationTime` (std::time_t) para tracking.
            
        - `topic` y `topicSetter` y `topicTime` if implement that. Métodos `setTopic(std::string newTopic, User* setter)` para encapsular storing and setting those fields.
            
- **Estructura global `ServerState`**: Implementar con:
    
    - `std::unordered_map<std::string, User*> users;`
        
    - `std::unordered_map<std::string, Channel*> channels;`
        
    - El comparador case-insensitive: En C++ `unordered_map` se puede customizar hash y equality. Implementar funciones hash que por ejemplo transformen a lower-case. O más simple: almacenar siempre las claves en una forma (e.g. all-lowercase keys). Decidir e implementar consistentemente (por ejemplo, transform nick a lower al insertar y buscar).
        
    - Mapa de command handlers: Ej. `std::unordered_map<std::string, std::function<void(User*, const std::vector<std::string>&)>> commandHandlers;`. Llenarlo en el constructor `ServerState()` mapeando "NICK"->&ServerState::handleNick, etc., or using lambdas capturing `this`.
        
    - Config variables: `int maxChannelsPerUser`, `std::string serverName`, `std::string networkName`, etc.
        
    - Métodos:
        
        - `getUser(const std::string& nick)` returns User* or nullptr.
            
        - `getOrCreateChannel(const std::string& name)` returns Channel* (creating if not exists).
            
        - `removeChannel(Channel* chan)` erases from map and deletes object.
            
        - `registerUser(User* user)` inserts into users map (with user->nickname as key).
            
        - `unregisterUser(User* user)` remove from map (and possibly free User).
            
        - `handleCommand(User* sender, Message& msg)` uses msg.command to lookup handler and calls it. If not found, calls default handler for unknown (which sends ERR_UNKNOWNCOMMAND).
            
        - For convenience, may implement wrappers to send messages: wrapping around global functions in Response module, passing serverName, etc.
            
- **Memoria y ownership**: Decidir cómo gestionar memoria de Users y Channels:
    
    - Podríamos usar punteros crudos y al final de la server loop limpiar, pero mejor usar smart pointers to avoid leaks on exceptions. Por ejemplo, `users` map could hold `std::shared_ptr<User>`. But careful with circular references (if User has list of Channel shared_ptr and Channel has list of User shared_ptr, that's circular). Alternatively:
        
        - Use raw pointers for cross-references but manage lifetime in one place. E.g. allocate User on heap when new connection, store in users map; free it when user quits. Allocate Channel when created, free when empty.
            
        - That means when removing user, also remove them from channels to avoid dangling pointer, then delete user.
            
        - Similarly for channel deletion, ensure no user has reference (user channel list must be updated).
            
        - This approach requires discipline but is straightforward.
            
        - Document: _A user always owns references to channels it’s in, and a channel does not own user (just observes). When a user is deleted, we must remove it from all channels to avoid dangling pointers._ And _when a channel is deleted (all users left), remove channel pointer from all users' channel lists? Actually if channel empty, no users should have it._ So that’s consistent.
            
    - Implement guard code: in `unregisterUser`, iterate user's channel list and call channel->removeMember(user) for each, then clear user's list. Then delete user.
        
    - In `removeMember` of Channel, after removing user, if channel becomes empty, call ServerState->removeChannel on itself.
        
    - Or use callbacks: channel->removeMember could check empty and call a callback (or hold pointer to server state) to remove itself from global map.
        
    - Ensure thread-safety if multi-threaded (likely not needed if one thread handles commands, but consider if using multiple threads for I/O and one for logic, might need locks on these maps). Initially assume single-thread for simplicity.
        
- **Final checks**:
    
    - Make sure to set initial values properly (e.g. isRegistered = false, no modes set by default, etc.).
        
    - Logging (if part of this phase, else skip to optional).
        
    - After implementing classes, write basic tests to ensure insertion, removal behave (this overlaps with unit tests later).
        
    - Efficiency: using unordered_map for users and channels yields average O(1) lookups, which is fine for typical loads. The number of channels a user can have is limited so iterating that on quit is fine.
        
    - Use case-insensitive compare for nick and channel: implement utility function `ircLowerCase(std::string)` (taking into account RFC case mapping: typically {}| are considered uppercase equivalents of [], but to keep it simple, we can tolower basic ASCII and that will suffice in most cases). Use that for keys in maps.
        
- **Handlers references**: Implement each command as a method `handleNick(User* u, const std::vector<std::string>& params)` inside ServerState or as free function receiving ServerState reference and user. Inside, use the structures to enforce logic from design. For example, in handleNick:
    
    - if params.empty(): sendNumeric(u, ERR_NONICKNAMEGIVEN).
        
    - else validate string, etc. (Call a utility isValidNick).
        
    - Check conflict: if (users.find(lowerNick) != end && users[lowerNick] != u) conflict.
        
    - etc.
        
    - If user not registered and this nick + user->username exists, complete registration by calling maybe a ServerState->finishRegistration(User* u) helper which sends welcome messages.
        
- **Mapping code to numeric**: maybe define constants or an enum for numeric codes to avoid using raw numbers in code (less magic numbers). E.g. `constexpr int ERR_NONICKNAMEGIVEN = 431;` etc., or define an enum class. Implementation can do either.
    
- **Edge cases**: e.g. what if two commands come in nearly simultaneously for same user (not likely since one thread reading sequentially). Or what if user changes nick to one differing only by case which our map might treat as same key – handle that carefully (if we tolower keys, "Nick" -> "nick" is same so conflict, which is correct because IRC nicks are case-insensitive). So code should treat it as conflict.
    
    - Possibly allow exactly same nick change (Nick to Nick) with no action or just ignore.
        
    - Also ensure server name is set and used in replies.
        
    - Use `serverName` in all sendNumeric and in server-sent notices.
        
- **Testing during implementation**: Write small scenarios (maybe in main or separate test file) to simulate adding user, renaming, joining channel etc., printing outcomes, to verify logic, before hooking to networking.
    

### Implementación del Parser LL(1)

- **Separación de líneas**: Desarrollar una función que reciba datos crudos (por ejemplo, desde `recv`) y los acumule en un buffer std::string. Ir buscando en ese buffer el patrón "\r\n". Cuando encuentre:
    
    - Extraer la subcadena desde inicio hasta antes de "\r\n".
        
    - Eliminar esa porción de la cadena (incluyendo el "\r\n") del buffer (o mantener índice de inicio).
        
    - Retornar la línea completa para parsear.
        
    - Repetir en loop por si múltiples mensajes llegaron en un solo paquete.
        
    - Si al final quedan datos sin CRLF, conservarlos para la próxima vez.
        
    - Esta función puede integrarse en la lógica de lectura del socket, pero conviene separarla para testear.
        
- **Tokenización**: Implementar una clase o struct `IRCLexer` que tome una línea (std::string) y tenga métodos para iterar tokens:
    
    - Podría pre-split manualmente en vector de tokens, o iterar carácter a carácter cada vez que se pide próximo token.
        
    - Simpler: manual parse:
        
        - If line begins with ':', output a token `PREFIX` = substring up to first space (excluding the leading ':'). Then position moves after that space.
            
        - Else, no prefix.
            
        - Next, parse the command token: gather characters until space or end. Mark if all digits or not (for possible numeric command detection).
            
        - Then while position < line.length():
            
            - If current char is space, skip all consecutive spaces.
                
            - If current char is ':', then the rest of the line (excluding the ':') is one trailing parameter token. Push that and break out.
                
            - Otherwise, parse a "middle" param: collect chars until next space or end. End at that position, push token.
                
            - Loop continues.
                
        - After loop, you'll have perhaps prefix token (maybe store separately rather than in list), one command token, and 0..15 parameter tokens.
            
    - The lexer can also directly produce a structured output instead of generic tokens: e.g. fill a struct with prefix string (if any), command string, and a vector<string> params. De hecho, eso suena más útil que tener un separate parser stage, dada la simplicity. Podríamos implementarlo as one combined parse function (which is fine since grammar is simple).
        
    - Manejar CRLF: by the time we call this, CRLF is removed. So the line likely ends at last param text char. Need to be careful: if trailing param is empty, line ends right after colon. Our loop logic should handle that (then trailing param token becomes empty string, which is acceptable).
        
- **Creación del objeto Message**: Implementar un struct `Message` con fields: `prefix` (string, empty if none), `command` (string), `params` (vector<string>). Perhaps also an enum or code for known command if we map it, but initially string is fine.
    
    - The parse function returns a `Message`. If error occurs (like totally empty line, or something weird), we might return a Message with command empty or an error flag. Or throw an exception ParseError which can be caught by the dispatcher to send appropriate error.
        
    - Simpler: if command empty (line was just prefix and nothing else), treat as error – we can directly handle by sending ERR_UNKNOWNCOMMAND or a generic parse error (there is no numeric for "wrong format" specifically besides 421 Unknown command, which might suffice). We can decide to use 421 for any badly parsed line that isn't a known command.
        
- **Incorporar limit 15 params**: En el parse, si contamos más de 15 params tokens:
    
    - Podemos agrupar extras en el último param (since grammar says at most 14 middle and 1 trailing). Por ejemplo, si el line has 16 spaces separated groups, the last one might be considered part of the previous trailing. Sin embargo, la ABNF estricto no lo permite. Lo más correcto: ignorar/eliminar cualquier param más allá del 15º, o guardar error.
        
    - Podríamos optar por truncar y perhaps log a warning. Muchos servidores simplemente truncarían. Pero como es guía y queremos robustez, podemos decidr truncar a 15 para no overflow buffers client.
        
- **Error handling**:
    
    - Si no hay comando token -> retornar un resultado marcando error. En la capa de arriba (handleCommand), si detecta msg.command vacío, puede enviar `421 ERR_UNKNOWNCOMMAND "" :Unknown command` o algún mensaje genérico de parse error.
        
    - Si se encontró comando pero más parámetros de los que el comando necesita, no es sintáctico error realmente. La validación de "needless params" no es usual en IRC (muchos comandos simplemente ignoran extras). Sin embargo, podríamos loggear si hay extras.
        
    - Si hay prefix pero no hay comando después de prefix (line like ":nick!u@h "), nuestro parse yields command = "". Entonces handle as unknown command scenario.
        
- **Integración con ejecutor**: Una vez parseado a Message, la función main loop del servidor llamará `serverState.handleCommand(user, message)`.
    
    - Si parse devolvió error flag, podría directamente enviar un error y no llamar a handleCommand normal (p.ej. if message.command empty and prefix present meaning error). Podríamos unify that as unknown command scenario.
        
- **Implementar y probar**:
    
    - Test strings:
        
        - ":irc.server 001 Nick :Welcome..." -> prefix "irc.server", command "001", params ["Nick", "Welcome..."].
            
        - "PING :12345" -> no prefix, command "PING", params ["12345"] (trailing).
            
        - "JOIN #chan key" -> command "JOIN", params ["#chan","key"].
            
        - "JOIN #chan1,#chan2 key1,key2" -> command "JOIN", params ["#chan1,#chan2","key1,key2"] (since comma is not space, it's part of param. Actually above logic would parse channel list as one param string containing commas; but for easier, we might not split by comma in parser. Instead, treat it as one param and let the command handler split by comma internally if needed. That’s fine.)
            
        - "PRIVMSG Bob :Hello world" -> command "PRIVMSG", params ["Bob","Hello world"].
            
        - ":Nick!user@host QUIT :Gone" -> prefix "Nick!user@host", command "QUIT", params ["Gone"].
            
        - Edge: "USER guest 0 * :Real Name" -> command "USER", params ["guest","0","*","Real Name"] (notice Real Name is trailing with spaces).
            
        - "USER guest 0 * :" -> trailing empty Real Name param, our parser should give "" as last param.
            
        - "NICK " -> command "NICK", no param (parser would yield params empty vector). That's not error at parse level, but handler will ERR_NEEDMOREPARAMS.
            
        - ":" -> just prefix and nothing else. Parser yields command empty -> handle as error.
            
    - Ensure the parser does not hang on partial input; that's done by line separation logic.
        
    - Efficiency: our parse is O(n) per message, which is fine given short lines (max 512 bytes).
        
    - Once confident, integrate in main server loop.
        

### Implementación de Manejadores de Comandos

Implementar cada handler diseñado, using the data structures and sending responses. Aquí destacamos los puntos críticos para cada uno:

- **Implementación `handleNick(User* u, const std::vector<std::string>& params)`**:
    
    - If `params.empty()`: `sendNumeric(u, 431)`. Return.
        
    - std::string newNick = params[0]; (Often trailing spaces trimmed by parser, so no need to trim).
        
    - Validate allowed chars: implement `isValidNick(const std::string&)` to check disallowed chars as per RFC[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Nicknames%20are%20non,the%20following%20restrictions). If fails: `sendNumeric(u, 432, {newNick})`.
        
    - Normalize case for lookup: std::string newKey = ircLowerCase(newNick).
        
    - If newKey == ircLowerCase(u->nickname): If user is already using that nick:
        
        - If user is not yet registered and is re-sending same nick as part of registration sequence, we might ignore (or still accept without change).
            
        - If user is registered and tries to change to same, possibly do nothing (send no error, or maybe still broadcast NICK? But broadcasting a nick change to same nick is silly). We can safely ignore and return.
            
    - Else if user with newKey exists in serverState.users:
        
        - If that existing user is the same pointer as u (should have been caught above though if we normalized same string), else another user: sendNumeric(u, 433, {newNick}) and return.
            
    - If passes:
        
        - Remove old nick from users map: `users.erase(ircLowerCase(u->nickname))`.
            
        - Save oldNick = u->nickname; set u->nickname = newNick.
            
        - Insert u into users map with key newKey.
            
        - If u->isRegistered:
            
            - Broadcast to all users on channels with u: iterate over u->channels list, for each channel's members, send `:<oldNick>!user@host NICK :<newNick>` to each (maybe reuse a function broadcastToChannel).
                
            - Also update any references: though likely none needed except map key replaced.
                
            - Append oldNick to u->nicknameHistory.
                
        - If u->isRegistered was false (still registering):
            
            - The nick is now set. Check if `u->username` is already set (from USER command) and registration not finished:
                
                - If yes, call `finishRegistration(u)` that sets isRegistered true and sends welcome (001-004 etc.).
                    
                - If no (USER not sent yet), then just wait.
                    
        - Log or print "Nick changed old->new".
            
- **Implementación `handleUser(User* u, const std::vector<std::string>& params)`**:
    
    - If `params.size() < 4`: `sendNumeric(u, 461, {"USER"})` (need more params). Return.
        
    - If `u->isRegistered`: sendNumeric(u, 462) (already registered). Return.
        
    - Extract `username = params[0]`, mode = params[1] (not used), unused = params[2], `realname = params[3]` (this param likely includes spaces already because parser will have trailing from ':' in original).
        
    - Assign `u->username = username`, `u->realName = realname`. We can ignore mode/unusued or store if needed for completeness (some servers use mode bit as flags for +i invisible on connection if mode & 8, etc., but not needed).
        
    - Mark that the user has sent USER (maybe store u->gotUserCommand = true).
        
    - If `u->nickname` is already set and not empty (the client likely sent NICK first):
        
        - Complete registration: call finishRegistration(u).
            
    - If not (maybe client sent USER first, without NICK yet):
        
        - Do nothing further; wait for NICK to arrive to complete registration.
            
    - finishRegistration(User*):
        
        - Set u->isRegistered = true.
            
        - send welcome numerics:
            
            - 001 RPL_WELCOME: text "Welcome to the Internet Relay Network <nick>!<user>@<host>"[alien.net.au](https://www.alien.net.au/irc/irc2numerics.html#:~:text=001%20%20RPL_WELCOME%20RFC2812%20%3AWelcome,Text%20varies%20widely) (we can incorporate actual network name if we have one, or just say "IRC Network").
                
            - 002 RPL_YOURHOST: "Your host is <servername>, running version <ver>".
                
            - 003 RPL_CREATED: "This server was created <date>" (could use compile date or config).
                
            - 004 RPL_MYINFO: "<servername> <version> <userModes> <chanModes>" listing supported mode flags.
                
            - Possibly 005 RPL_ISUPPORT with token features (optional).
                
            - 375/372/376 for MOTD start, lines, end if MOTD file present.
                
            - These strings should be prepared in some config or constants.
                
        - After these, user can join channels etc.
            
- **Implementación `handleQuit(User* u, const std::vector<std::string>& params)`**:
    
    - Extract message = params.empty()? default "Quit": params[0].
        
    - For each channel in a copy of u->channels list:
        
        - Channel* chan = it;
            
        - chan->removeMember(u); // inside this, if empties channel, it will remove from serverState and delete chan possibly.
            
        - When removeMember broadcasts PART or QUIT to others? Actually:
            
            - We might choose to broadcast a QUIT differently. In IRC, the QUIT message is network-wide and includes all channels, so sending one QUIT per channel is inefficient and can produce duplicates for users in multiple channels with the quitter. Instead:
                
            - Strategy: Collect all users who share at least one channel with u (we can iterate u->channels to get each channel's members, add them to a set of targets). Remove u itself from that set.
                
            - Then send **one** QUIT message to each user in that set (each user gets one quit notice).
                
            - That QUIT message prefix = u (old nick, etc.), content = quit message.
                
            - This avoids duplicates. Implement this approach.
                
        - Implementation:
            
            - Create an empty set<User*> targets;
                
            - For each channel in u->channels: for each member m in channel->members: if m != u, insert m in targets.
                
            - Then for each target in targets: send `:<u->nickname>!<u->username>@<u->hostname> QUIT :<message>` to them.
                
        - Now proceed to remove u from each channel: we did removeMember above which likely removed them. But careful: if removeMember triggers deletion of channel, it might attempt to broadcast a PART. We might want to avoid broadcasting PART in removeMember if it's being called due to QUIT, because we'll broadcast QUIT ourselves.
            
            - Solution: add a parameter to removeMember like `isQuit = false`; if true, skip broadcasting part because a QUIT will cover it. Or simply handle broadcasting outside entirely:
                
            - Simpler: do **not** broadcast PART for normal part in removeMember, instead always handle broadcasting of leaving events in handlers:
                
                - For PART command, after removeMember, we broadcast PART.
                    
                - For QUIT command, we broadcast QUIT outside removeMember.
                    
            - That might be cleaner design (channel->removeMember just mutates state, doesn't send). Let's do that.
                
        - After removal from all channels, user->channels list is empty.
            
    - If we have any server-side resources (like in a more advanced server, maybe user was an operator or had some connection state), handle those (not much here).
        
    - Remove user from serverState.users map and delete User object (or mark for deletion if in loop).
        
    - Actually, careful: This handler likely called from within a loop iterating user list or from network thread. If we delete user here and then try to send messages to them (like maybe sending an ACK), that would be wrong. But we typically do not send anything to the user on quit except possibly an ACK if it was client-initiated (some servers echo the quit message? Actually no, they don't send anything to the quitter except closing the connection).
        
    - So safe to delete after broadcasting to others.
        
    - Closing socket: after calling handleQuit, the network layer will close it.
        
- **Implementación `handleJoin(User* u, const std::vector<std::string>& params)`**:
    
    - If params.empty(): sendNumeric(u, 461, {"JOIN"}). return.
        
    - std::string channelsParam = params[0]; std::string keysParam = params.size()>1 ? params[1] : "";
        
    - Split channelsParam by commas into a list (e.g. using find(',')). Same for keysParam.
        
    - For each channelName in channels list:
        
        - std::string key = (matching key from keys list if available, else "").
            
        - Validate name: use a function isValidChannelName(name) to check prefix #/&/+/!, length <=50, and no spaces or commas or control G or colon[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=Channels%20names%20are%20strings%20,Channel%20names%20are%20case%20insensitive). If fails: sendNumeric(u, 403, {name}) and continue to next (since no such channel).
            
        - If u->channels.size() >= maxChannelsPerUser: sendNumeric(u, 405, {name})[irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=405%20%20%20%20,try%20to%20join%20another%20channel) and continue (can't join more).
            
        - Find or create Channel:
            
            - Channel* chan;
                
            - if not exists in serverState.channels:
                
                - chan = new Channel(name); serverState.channels[ircLower(name)] = chan;
                    
                - set chan->creator = u (if safe channel maybe).
                    
                - _Important:_ Add the user _after_ creation.
                    
            - else: chan = serverState.channels[...]
                
        - If channel exists:
            
            - If user already in chan: ignore (no need to rejoin). Possibly continue to next channel in list.
                
            - Check bans: iterate chan->banList, if any maskMatches(u->username, u->hostname, u->nickname) and not matches any in chan->exceptionList:
                
                - sendNumeric(u, 474, {name});
                    
                - continue.
                    
            - Check invite-only: if chan->inviteOnly and u not in chan->inviteList:
                
                - sendNumeric(u, 473, {name});
                    
                - continue.
                    
            - Check key: if chan->hasKey:
                
                - if provided key != chan->key: sendNumeric(u, 475, {name}); continue.
                    
                - else (key correct) -> proceed.
                    
            - Check limit: if chan->userLimit > 0 and chan->members.size() >= chan->userLimit:
                
                - sendNumeric(u, 471, {name}); continue.
                    
        - Now safe to join:
            
            - chan->addMember(u). If chan was newly created (we know because we created above or maybe check chan->members was empty before add):
                
                - mark u as operator in that channel (pass asOp=true or separate call chan->setOperator(u)).
                    
                - If channel is '!' safe: set chan->creator = u and chan->setMode('O', true, u) (though 'O' is not something user sees).
                    
                - Possibly also set default channel modes: many networks auto-set +nt on new channels. If we do that: chan->moderated = false, chan->noExtMsg = true (maybe set +n), chan->topicLock = true (set +t) as defaults. This is optional network policy. Could skip or implement as config option.
                    
            - On join, remove user from chan->inviteList if present (so that invite is single-use).
                
            - Add channel to u->channel list (u->joinChannel(chan)).
                
            - Prepare broadcast join: for each member in chan (excluding u?), typically the new join is broadcast to others. Implementation detail: Actually, clients expect to see their own join as well, so the server typically sends the JOIN message to _all_ current members including the new user. So we'll broadcast to all in chan:
                
                - For each member m in chan->members: send `:<u->nick>!<u->user>@<u->host> JOIN <name>` to m.
                    
            - After broadcasting:
                
                - If this channel was newly created or has no topic: sendNumeric(u, 331, {name}) "No topic".
                    
                - Else if existing with topic: sendNumeric(u, 332, {name, topic}).
                    
                - If we track who set topic/time, optionally sendNumeric(u, 333, {...}).
                    
                - Send NAMES list: we can reuse the NAMES handler logic for a single channel:
                    
                    - Construct the names line with appropriate prefixes. Possibly break if too long.
                        
                    - sendNumeric(u, 353, { "=", name, ":" + names });
                        
                    - sendNumeric(u, 366, { name });
                        
                - Done for that channel.
                    
            - If channel was created new:
                
                - Optionally log "Channel #name created by nick".
                    
        - Continue with next channel in list.
            
    - Complexity: mostly straightforward.
        
    - Concurrency: if multiple join same time, not likely since single thread.
        
    - After processing all, return.
        
- **Implementación `handlePart(User* u, const std::vector<std::string>& params)`**:
    
    - If params.empty(): sendNumeric(u, 461, {"PART"}); return.
        
    - std::string channelsParam = params[0]; optional message = params.size()>1 ? params[1] : "";
        
    - Split channelsParam by comma.
        
    - For each name:
        
        - if channel not in serverState.channels: sendNumeric(u, 403, {name}); continue.
            
        - Channel* chan = ...
            
        - If !chan->hasMember(u): sendNumeric(u, 442, {name}); continue.
            
        - Otherwise:
            
            - chan->removeMember(u). (Do not broadcast inside removeMember; we'll do it after adjusting state.)
                
            - In removeMember: remove from members, if empty => removeChannel.
                
            - After removal, if channel still exists (not empty):
                
                - Broadcast PART: to all remaining in chan _and_ to u themselves? Actually, since u parted intentionally, the client expects to see the PART message as confirmation too (especially if parted at server's behest or multiple clients same user).
                    
                - So yes, send `:<u->nick>!user@host PART <channel> :<message>` to:  
                    for each member in oldMembers (the removeMember could give us list or we get from before removal):  
                    Actually better to gather recipients before removal to include all including leaving user (which might have been removed from list now).  
                    We can simply:
                    
                    - Make a copy of chan->members _before_ calling removeMember.
                        
                    - Then call removeMember.
                        
                    - Then send PART to everyone in that copy including the user (the user is still in copy).
                        
                    
                    - But if removeMember deletes channel (empty), then we need to handle differently:  
                        If channel is empty after removal, we typically don't broadcast PART to the leaving user themselves because there's no one left to broadcast from server side. But to be correct, even if channel empties, the user should still get the PART message (some servers might skip if user parted themselves? Actually no, the user still should see they parted).  
                        We'll handle uniformly: if copy list is not empty (it contains at least the user who left), we send to all in copy.  
                        If channel is empty, copy had only that user possibly; we'll still send them the PART echo.
                        
                    
                    - (Alternatively, we could send to user separately if needed).
                        
                - Format the part message text: if message param not given, some clients show a default. We can just not include trailing part message if not given (so it ends with channel without colon part). But per RFC, the part message is optional trailing param. If none given, we may simply not put the " :" part. Implementation: if message is empty, omit the " :message" segment.
                    
            - Remove channel from user's list.
                
            - If channel destroyed, nothing more to do for it (no broadcast needed beyond perhaps the echo).
                
    - End loop.
        
- **Implementación `handleMode(User* u, const std::vector<std::string>& params)`**:
    
    - If params.empty(): sendNumeric(u, 461, {"MODE"}); return.
        
    - std::string target = params[0];
        
    - bool isChannel = isChannelName(target) (e.g. startswith #/&/!/+) ;
        
    - if (isChannel):
        
        - Channel* chan = find channel in map (ircLowerCase(target)).
            
        - If not found: sendNumeric(u, 403, {target}); return.
            
        - If params.size() == 1:
            
            - Query current modes:
                
                - Build mode string like "+ntk", and gather mode parameters like key and limit if those modes set.
                    
                - sendNumeric(u, 324, {target, modeString + (params if any)}). e.g. if +k and +l: "324 user #chan +kl 7 keypassword".
                    
                - sendNumeric(u, 329, {target, std::to_string(chan->creationTime)}) if we have that.
                    
            - return.
                
        - Else:
            
            - The mode change string = params[1], and subsequent params possibly in params[2..].
                
            - If user not in channel: sendNumeric(u, 442, {target}); return (you can't change modes if not on channel).
                
            - If user is in channel but not operator: sendNumeric(u, 482, {target}); return.
                
            - Now parse the mode changes:
                
                - std::string modeChanges = params[1];
                    
                - Use an index j for param index, starting at 2 (next param in params list).
                    
                - char sign = '\0';
                    
                - std::string outputModes; // to build the composite string to broadcast
                    
                - std::vectorstd::string outputParams; // corresponding params for those modes that need it.
                    
                - For each char c in modeChanges:
                    
                    - if (c == '+' or c == '-') { sign = c; continue; }
                        
                    - If sign is '\0', it means no sign before (should default to '+' by convention or ignore until sign appears; most implementations treat initial no sign as '+'. We'll assume modeChanges always starts with +/-. If not, we can default sign='+' at start).
                        
                    - bool adding = (sign == '+');
                        
                    - switch(c):
                        
                        - case 'o': case 'v':  
                            ~ if adding:  
                            if j >= params.size(): sendNumeric(u, 461, {"MODE"}); break out (missing parameter).  
                            std::string nick = params[j++];  
                            User* targetUser = getUser(nick);  
                            if (!targetUser or !chan->hasMember(targetUser)) {  
                            // If the user to op/voice is not in channel, error:  
                            sendNumeric(u, 441, {nick, target});  
                            break; // skip this mode change.  
                            }  
                            if (c == 'o'): chan->opSet.insert(targetUser) (or chan->members[targetUser] |= OP_FLAG);  
                            else if (c == 'v'): chan->voiceSet.insert(targetUser);  
                            outputModes += (adding? "+" : "-"); outputModes += c;  
                            outputParams.push_back(nick);  
                            ~ if !adding (removing):  
                            similar, but remove from sets. Also require param present (should be).
                            
                        - case 'i': case 't': case 'n': case 'm': case 'p': case 's': case 'r': case 'q': case 'a':  
                            ~ no param needed. If adding: set flag true; removing: set false. (For 'q' and 'a', consider if allowed: maybe if c == 'q' and adding, we might restrict? But let's allow if op tries, though RFC says quiet 'q' is server-only. We could simply ignore 'q' attempts from user by sending unknownmode or nop).  
                            outputModes += sign; outputModes += c;  
                            // no param in outputParams
                            
                        - case 'k':  
                            ~ if adding:  
                            if j >= params.size(): sendNumeric(u, 461, {"MODE"}); break out.  
                            std::string key = params[j++];  
                            if (chan->hasKey && chan->key == key && chan->key != "") {  
                            // key already set to same, do nothing; (some servers still show a change? We'll do nothing to avoid duplication)  
                            } else {  
                            chan->key = key; chan->hasKey = true;  
                            outputModes += "+k"; outputParams.push_back(key);  
                            }  
                            if removing:  
                            chan->hasKey = false; chan->key = "";  
                            outputModes += "-k";  
                            // Note: no param for -k in output.
                            
                        - case 'l':  
                            ~ if adding:  
                            if j >= params.size(): sendNumeric(u, 461, {"MODE"}); break out.  
                            std::string limitStr = params[j++];  
                            int limit = std::stoi(limitStr);  
                            if (limit < 0) limit = 0;  
                            if (limit > 0) {  
                            chan->userLimit = limit; chan->limitActive = true;  
                            outputModes += "+l"; outputParams.push_back(limitStr);  
                            } else {  
                            // if limit given as 0, treat as removing limit  
                            chan->limitActive = false;  
                            outputModes += "-l";  
                            }  
                            if removing:  
                            chan->limitActive = false;  
                            outputModes += "-l";  
                            // no param.
                            
                        - case 'b': case 'e': case 'I':  
                            ~ if adding:  
                            if j >= params.size():  
                            // If no param and adding, it might be a request to list, not a missing param error  
                            // Check if they intended to list: we detect if adding and no param at all in the command input for this mode.  
                            if (modeChanges.size() == 2 && modeChanges[0] == '+' && modeChanges[1] == c && params.size() <= 2) {  
                            // It's like "MODE #chan +b" with no param after mode string  
                            // Provide list output:  
                            if (c == 'b') { for each mask in chan->banList: send 367; then 368. }  
                            if (c == 'e') { ... 348/349. }  
                            if (c == 'I') { ... 346/347. }  
                            } else {  
                            sendNumeric(u, 461, {"MODE"});  
                            }  
                            break out of loop (stop processing further modes).  
                            }  
                            std::string mask = params[j++];  
                            // maybe validate mask?  
                            std::vectorstd::string& list = (c=='b'? chan->banList : c=='e'? chan->exceptionList : chan->inviteList);  
                            // avoid duplicates:  
                            if (std::find(list.begin(), list.end(), mask) == list.end()) {  
                            list.push_back(mask);  
                            }  
                            outputModes += "+";  
                            outputModes += c;  
                            outputParams.push_back(mask);  
                            if removing:  
                            if (j >= params.size()) {  
                            // If removing with no param, maybe list? But listing on -b is not a thing. So just error:  
                            sendNumeric(u, 461, {"MODE"}); break out.  
                            }  
                            std::string mask = params[j++];  
                            std::vectorstd::string& list = choose same as above;  
                            auto it = std::find(list.begin(), list.end(), mask);  
                            if (it != list.end()) list.erase(it);  
                            outputModes += "-"; outputModes += c;  
                            outputParams.push_back(mask);
                            
                        - default:  
                            ~ Unknown mode char: sendNumeric(u, 472, {std::string(1,c)}). Continue loop (skip to next char, or break out entirely?  
                            Possibly just skip it.)
                            
                    - End switch
                        
                - End for
                    
            - If outputModes not empty:
                
                - Compose a single MODE message to broadcast:  
                    std::string fullMode = outputModes;  
                    For each param in outputParams, append " " + param to fullMode (taking care of spacing).
                    
                - Broadcast to channel members: for each member m in chan:  
                    send `:<u->nick>!user@host MODE <channel>` + fullMode.
                    
            - (If outputModes is empty, means either nothing changed or all changes were invalid; no broadcast.)
                
            - Note: If multiple sign groups were in input, our outputModes might still represent them combined or separate. We combined sequential, which should be fine. If some modes failed, others succeeded, we still broadcast those that succeeded.
                
            - Ensure voice/op changes reflect in our channel data structures properly (the logic above does that).
                
            - Possibly also reply to user for certain mode changes:
                
                - Actually in IRC, setting a mode usually only yields the broadcast, not a direct numeric to the setter (except the echo itself is seen by them if they are in channel).
                    
                - For user mode changes, many servers echo back with either a numeric or a simple confirmation. But for channels, no direct numeric aside from broadcast.
                    
            - If any error needed to be sent (like 401 if target not on channel, or 467 key already set? Actually some networks have 467 ERR_KEYSET if try to +k when already has one; we could implement that:  
                if adding +k and chan already hasKey, some IRCD require removing old key first or use -k to change. We could send 467 "<channel> :Channel key already set". But not mandatory. We might allow override key for simplicity.
                
                - or ERR_NOCHANMODES (477) if channel type doesn't support modes? Not needed if all have modes.
                    
                - Keep it simple with essentials.
                    
        - (we covered listing bans with +b with no param above).
            
        - Ensure to handle index j carefully and sign persistence.
            
    - if (!isChannel):
        
        - targetNick = target.
            
        - if (targetNick != u->nickname) {  
            if (!u->isIRCOperator) { sendNumeric(u, 502); return; }  
            // If we had IRC operators, they'd be allowed to change others? Actually not standard, aside from oper override of +i etc.  
            // We'll assume no, just 502.  
            }
            
        - If params.size() == 1:  
            // query own modes  
            std::string umodes = u->getUserModeString(); // e.g. "+i"  
            sendNumeric(u, 221, {umodes});  
            return;
            
        - Else:  
            std::string modeStr = params[1];  
            char sign = '\0';  
            for c in modeStr:  
            if (c=='+'||c=='-'){ sign=c; continue; }  
            bool adding = (sign != '-'); // default '+' if no sign encountered.  
            switch(c):  
            - 'i': if adding: u->invisible = true; else u->invisible = false; break;  
            - 'w': if adding: u->wallops = true; else u->wallops = false; break;  
            - 'o': if adding:  
            // user trying to set themselves IRCop, not allowed via MODE.  
            sendNumeric(u, 481) ("No privilege"); continue or break out.  
            else:  
            // They can't remove their op status via MODE either; it's set by server.  
            sendNumeric(u, 481); continue.  
            - other: sendNumeric(u, 501, {std::string(1,c)}); // unknown user mode flag.  
            // no further params needed for user modes in RFC 2812.  
            end for  
            // After applying, optionally inform user:  
            std::string newModes = u->getUserModeString();  
            // We can send either a numeric 221 or echo:  
            sendNumeric(u, 221, {newModes}); // Many servers just send updated mode in a 221 or a raw "MODE nick +i".  
            // Actually RFC says 221 is used for query, not sure if to confirm change. Alternatively, we can send the usual "MODE nick +i" message with user prefix:  
            sendMessage(u, u, "MODE", {u->nickname, newModes});  
            // But since it's the same user, sending to them with their own prefix might be weird. Numeric is fine.
            
- **Implementación `handleTopic(User* u, const std::vector<std::string>& params)`**:
    
    - If params.empty(): sendNumeric(u, 461, {"TOPIC"}); return.
        
    - std::string channelName = params[0];
        
    - Channel* chan = find channel; if !chan: sendNumeric(u, 403, {channelName}); return.
        
    - bool settingTopic = (params.size() > 1);
        
    - if (!chan->hasMember(u)):
        
        - if (settingTopic): sendNumeric(u, 442, {channelName}); return.
            
        - else (just querying):
            
            - if chan->secret or chan->private: sendNumeric(u, 442, {channelName}); return (can't view).
                
            - else allow query (some nets allow seeing topic if not in channel but channel visible; we could allow for public channels).
                
    - if (settingTopic):
        
        - if (chan->topicOpOnly && !chan->isOperator(u)): sendNumeric(u, 482, {channelName}); return.
            
        - std::string newTopic = params[1]; // trailing, may contain spaces, can be empty string which means clear topic.
            
        - If newTopic is empty:
            
            - chan->topic.clear();
                
            - chan->hasTopic = false (if we track).
                
            - chan->topicSetter = u or "" (clear), chan->topicTime = now.
                
        - else:
            
            - chan->topic = newTopic; chan->hasTopic = true;
                
            - chan->topicSetter = u->nickname (or user ID), chan->topicTime = now.
                
        - Broadcast: for each member m in chan:  
            send `:<u->nick>!user@host TOPIC <channel> :<topic>` (if topic is empty, it will appear as "TOPIC #chan :" with no text after colon, which clients interpret as topic removal).
            
        - No numeric reply to requestor specifically aside from the echo.
            
    - else (querying topic):
        
        - if (chan->topic.empty()): sendNumeric(u, 331, {channelName});
            
        - else: sendNumeric(u, 332, {channelName, chan->topic});  
            if we have topicTime: sendNumeric(u, 333, {channelName, chan->topicSetter, std::to_string(chan->topicTime)});
            
- **Implementación `handleKick(User* u, const std::vector<std::string>& params)`**:
    
    - If params.size() < 2: sendNumeric(u, 461, {"KICK"}); return.
        
    - std::string channelName = params[0]; std::string targetNick = params[1];
        
    - std::string comment = params.size() > 2 ? params[2] : "";
        
    - Channel* chan = find channel; if !chan: sendNumeric(u, 403, {channelName}); return.
        
    - if (!chan->hasMember(u)): sendNumeric(u, 442, {channelName}); return.
        
    - if (!chan->isOperator(u)): sendNumeric(u, 482, {channelName}); return.
        
    - User* targetUser = getUser(targetNick);
        
        - If !targetUser or !chan->hasMember(targetUser): sendNumeric(u, 441, {targetNick, channelName}); return.
            
    - Perform removal: chan->removeMember(targetUser).
        
    - Prepare KICK message: `:<u->nick>!user@host KICK <channel> <targetNick> :<reason>`
        
        - If comment not provided, we could set reason = u->nickname (some implementations do that), or just an empty trailing (which results in " :").
            
        - We might do: if comment == "" then reason = u->nickname.
            
    - Broadcast KICK:
        
        - For each member remaining in chan (after removal) plus the targetUser:
            
            - Send the KICK message. (Even though targetUser is no longer in chan members after removal, we still want to send to them).
                
        - Actually, since we have targetUser pointer, just explicitly send to targetUser as well.
            
    - If chan is now empty after removal: serverState.removeChannel(chan). (chan->removeMember might have done it already, depending on implementation).
        
    - Remove channel from targetUser->channels list if that method isn't called in removeMember.
        
    - The kicker remains in channel (if not kicking themselves).
        
    - If targetUser was the last operator or important in channel, we do nothing special (channel can exist without ops).
        
    - If multiple targets or channels passed (not implemented unless we want):
        
        - Could parse channelName list and targetNick list by splitting by commas (the command allows "KICK #chan1,#chan2 user1,user2 reason"). If we wanted to implement:
            
            - If multiple channels and one target or equal count, they map one-to-one.
                
            - For simplicity, consider only one target as typical usage.
                
- **Implementación `handleInvite(User* u, const std::vector<std::string>& params)`**:
    
    - If params.size() < 2: sendNumeric(u, 461, {"INVITE"}); return.
        
    - std::string targetNick = params[0]; std::string channelName = params[1];
        
    - Channel* chan = find channel; if !chan: sendNumeric(u, 403, {channelName}); return.
        
    - if (!chan->hasMember(u)): sendNumeric(u, 442, {channelName}); return.
        
    - if (chan->inviteOnly && !chan->isOperator(u)): sendNumeric(u, 482, {channelName}); return.
        
    - User* targetUser = getUser(targetNick); if (!targetUser): sendNumeric(u, 401, {targetNick}); return.
        
    - if (chan->hasMember(targetUser)): sendNumeric(u, 443, {targetNick, channelName}); return.
        
    - // All clear:
        
    - chan->inviteList.push_back(targetUser->nickname); // store by nickname (could store mask or pointer, but pointer can become invalid if user leaves network, though if they leave, invite moot. Still storing nick is fine).
        
    - sendNumeric(u, 341, {channelName, targetNick});
        
    - sendMessage(u, targetUser, "INVITE", {targetUser->nickname, ":" + channelName});
        
        - Actually, format typically: prefix= inviter, `INVITE <targetNick> <channel>` (target sees that). But some docs show `:<inviter> INVITE <targetNick> :<channel>`, need to confirm correct order:
            
        - According to RFC, the server sends to target: `:InviterNick!user@host INVITE <targetNick> :#channel`.
            
        - We'll implement that format.
            
    - (No broadcast to channel needed.)
        
- **Implementación `handleNames(User* u, const std::vector<std::string>& params)`**:
    
    - if params.empty():
        
        - For each channel in serverState.channels:
            
            - if (chan->secret or chan->private) and !chan->hasMember(u): skip (do not list).
                
            - else prepare names list:
                
                - std::string names = "";
                    
                - for each member in chan->members:  
                    if member->isOperator(chan): names += "@" + member->nickname;  
                    else if member->isVoice(chan): names += "+" + member->nickname;  
                    else names += member->nickname;  
                    names += " ";
                    
                - trim trailing space.
                    
                - char prefix = chan->secret? '@': chan->private? '*': '=';
                    
                - sendNumeric(u, 353, {std::string(1,prefix), chan->name, ":" + names});
                    
        - After listing all channels, also consider listing "users not on any channel":
            
            - IRC often lists them as channel "*". We can implement:
                
                - Collect users that are not in any visible channel (i.e., either in no channel or only in secret ones they are not in? Actually from perspective, maybe list all users who share no channel with others? But traditional /names with no param _does_ list all users (including those who are invisible? Actually invisible +i users are not listed unless you share a channel or are an oper).
                    
                - We might skip this complexity.
                    
            - We can at least send an `366 RPL_ENDOFNAMES * :End of NAMES list`.
                
        - sendNumeric(u, 366, {"*", ":End of NAMES list"}).
            
    - else:
        
        - Split params[0] by comma into channel names.
            
        - For each name:
            
            - Channel* chan = find channel by name ignoring case.
                
            - if (!chan || (chan->secret||chan->private) && !chan->hasMember(u)):  
                sendNumeric(u, 366, {name, ":End of NAMES list"}); // no names.  
                else:  
                build names string as above.  
                prefix = '=' or as above for type.  
                sendNumeric(u, 353, {std::string(1,prefix), chan->name, ":" + names});  
                sendNumeric(u, 366, {chan->name, ":End of NAMES list"});
                
- **Implementación `handlePrivmsg(User* u, const std::vector<std::string>& params, bool notice)`**:
    
    - if params.empty() or params.size()<2 (target and message needed):
        
        - if (!notice) {  
            if params.empty(): sendNumeric(u, 411, {"PRIVMSG"});  
            else: sendNumeric(u, 412, {});  
            }  
            return;
            
    - std::string targetList = params[0];
        
    - std::string message = params[1]; (the trailing param)
        
    - Split targetList by comma into targets.
        
    - For each target in targets:
        
        - if (target looks like channel name (# etc)):  
            Channel* chan = find.  
            if (!chan || (chan->secret && !chan->hasMember(u))):  
            if (!notice) sendNumeric(u, 401, {target});  
            continue;  
            if (chan->moderated && !chan->isOperator(u) && !chan->isVoice(u)):  
            if (!notice) sendNumeric(u, 404, {target});  
            continue;  
            if (chan->noExtMsg && !chan->hasMember(u)):  
            if (!notice) sendNumeric(u, 404, {target});  
            continue;  
            // If notice and those conditions fail, just drop (don't continue with error).  
            // If user is banned in channel and not member:  
            // (This is an optional check, not in RFC as separate numeric but could block):  
            // We skip unless implementing +b as quiet ban which is extension.  
            // So ignore ban for sending if they're not in channel because +n covers external.  
            // If they're in channel and banned (which means an op banned them but didn't kick?), usually +b doesn't block speaking unless +m and not voiced, so handled by moderated.  
            // So nothing additional.  
            // Now deliver:  
            std::string fullMsg = ":" + u->nickname + "!" + u->username + "@" + u->hostname + (notice? " NOTICE " : " PRIVMSG ") + target + " :" + message;  
            for each member in chan->members:  
            if (member == u) continue; // skip sender for echo  
            sendRaw(member, fullMsg);
            
        - else (target is a user nick or maybe server mask like $_. If nick):  
            User_ dest = getUser(target);  
            if (!dest || dest->isQuitPending):  
            if (!notice) sendNumeric(u, 401, {target});  
            continue;  
            if (!notice) {  
            // If dest->awayMessage set: sendNumeric(u, 301, {dest->nickname, dest->awayMessage});  
            }  
            // Form message:  
            std::string fullMsg = ":" + u->nickname + "!" + u->username + "@" + u->hostname + (notice? " NOTICE " : " PRIVMSG ") + dest->nickname + " :" + message;  
            sendRaw(dest, fullMsg);  
            // No response needed from dest side beyond maybe an auto-reply if a bot, but that's outside server.
            
        - If target is a server mask (starts with '$' or something), we are not implementing that, could just ignore or treat as invalid target leading to 401.
            
    - End loop targets.
        
    - There is also ERR_TOOMANYTARGETS (407) if they target too many? It's rarely used except if a user target resolves to multiple (like user@host with many matches). We skip that.
        
- **Implementación `handlePing(User* u, const std::vector<std::string>& params)`**:
    
    - If params.empty(): sendNumeric(u, 409, {}); return.
        
    - std::string token = params[0]; // could be server name or random.
        
    - We ignore everything and just respond:
        
    - sendRaw(u, ":" + serverName + " PONG " + serverName + " :" + token);
        
    - (The format: if client sent "PING <stuff>", server replies "PONG <ourservername> :<stuff>"[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Most%20messages%20sent%20from%20a,for%20both%20errors%20and%20normal), including the colon for token if needed. Actually, minimal for a single param token can be without colon if no spaces in token. But to be safe, always put ":" before the token in PONG.)
        
    - Mark lastPing time if tracking.
        
- **Implementación `handlePong(User* u, ...)`**:
    
    - The client responding to our PING. We can just ignore or use it to clear a ping flag. No immediate reply.
        
- **Implementación `handleUnknown(User* u, Message& msg)`**:
    
    - sendNumeric(u, 421, {msg.command}); // ERR_UNKNOWNCOMMAND "<command> :Unknown command"
        
    - Perhaps log unknown commands.
        
- **Integration**:
    
    - The server's main loop receiving messages will:
        
        - parse the raw into Message,
            
        - if parse errors (like message.command empty due to misformat), it might call unknown or ignore,
            
        - else find handler in commandHandlers map (with key as uppercase of msg.command because we should match case-insensitively).
            
            - We can transform msg.command to uppercase.
                
            - If it's numeric (e.g. "001"), those won't come from clients usually; if they do, treat as unknown (unless we want to parse server to server messages, which we don't).
                
        - If found, call it with user and msg.params.
            
        - If not found, call handleUnknown.
            
        - Each handler uses sendNumeric or sendRaw to communicate.
            
- **Send functions**: Implement `sendRaw(User* dest, const std::string& msg)` to queue the message to that user's socket send buffer, including "\r\n". Or directly send if not buffering.
    
    - Possibly integrate with a networking layer that handles actual send later.
        

### Implementación de Infraestructura de Respuestas

- **Definir constantes**: In a header, define numeric codes:
    
    - e.g. `#define RPL_WELCOME 1` or 001 (leading zeros invalid in C, could define as 1 and always format as 3 digits when sending).
        
    - Or an enum { RPL_WELCOME = 001, ... } but 001 is octal in C++ (1 would be correct).
        
    - Could define as strings: `const std::string RPL_WELCOME = "001";` which might be easier to embed in messages.
        
    - Or simply use numeric int and convert to string when sending with std::setw(3).
        
    - We'll do probably use strings in code directly for simplicity (less robust, but okay in a controlled environment).
        
- **Implement sendNumeric**: e.g.
    
    `void ServerState::sendNumeric(User* u, int code, std::vector<std::string> params = {}) {     char codeStr[4];     sprintf(codeStr, "%03d", code);     std::string msg = ":" + serverName + " " + codeStr + " " + u->nickname;     for (size_t i = 0; i < params.size(); ++i) {         // If it's last param and contains space or is empty, prefix with ':'         bool last = (i == params.size()-1);         if (last) {             msg += " :";             msg += params[i];         } else {             msg += " ";             msg += params[i];         }     }     msg += "\r\n";     u->sendBuffer += msg; // or send directly. }`
    
    - This uses u->nickname as target. Ensure user has nick (if not registered, some errors maybe still send "_" or something as target; RFC says if not registered and you error, you can omit target or use "_". We could implement: if (!u->isRegistered) use "*" as target in numeric).
        
    - Use similar logic in broadcast: we might send numeric to multiple but numeric replies usually to one user.
        
- **Implement sendMessage**: for non-numeric:
    
    `void ServerState::sendMessage(const std::string& prefix, User* dest, const std::string& command, const std::vector<std::string>& params) {     std::string msg = ":" + prefix + " " + command;     for (size_t i=0; i<params.size(); ++i) {        if (i == params.size()-1) { msg += " :" + params[i]; }        else { msg += " " + params[i]; }     }     msg += "\r\n";     dest->sendBuffer += msg; }`
    
    - Often, prefix will be either a serverName (for numeric and notices) or user->getPrefix() for user-sourced messages. We can have a helper `u->getPrefix()` returning "nick!user@host".
        
    - Use this for invites, etc.
        
- **Mapping of code to text**: Optionally maintain a map: e.g.  
    `std::map<int, std::string> errMessages;` fill it like:
    
    - errMessages[401] = "<target> :No such nick/channel";
        
    - errMessages[433] = "<nick> :Nickname is already in use";
        
    - etc.
        
    - Then sendNumeric could do:
        
        - auto it = errMessages.find(code); if(it != end):
            
            - format = it->second,
                
            - find placeholders like <nick> or <channel> in format and replace with actual param if provided.
                
        - This is neat but perhaps overkill for our implementation.
            
        - We might just directly use sendNumeric with explicit param assembly in each handler.
            
    - However, keeping the exact text from RFC helps uniform output. Perhaps we can partially implement:
        
        - Keep static map of code to base text.
            
        - Then in sendNumeric, if vector params is empty, we might need to fill from context which we may not easily have here. So probably leave it to handler to provide correct param substitution.
            
    - So a simpler approach: Hardcode the reply formatting at call site:
        
        - E.g. in handleNick conflict: `sendNumeric(u, 433, {newNick, ":Nickname is already in use"});`
            
        - Or define a helper: `sendErrNicknameInUse(u, newNick)` that calls sendNumeric(...).
            
        - Could be verbose but ensures we include all needed info and localized text.
            
    - We'll choose to explicitly include the colon and message in params for each error to ensure it's correct as per RFC, since in many numerics the format after the target is `:<text>`.
        
        - E.g. `sendNumeric(u, 403, {channel, ":No such channel"});`
            
        - `sendNumeric(u, 442, {channel, ":You're not on that channel"});`
            
        - etc.
            
- **Testing the sending**: Simulate a scenario to ensure formatting:
    
    - If Nick in use:
        
        - Expect `:server 433 NewNick :Nickname is already in use`.  
            Actually, according RFC 1459: `433 <nick> <newnick> :Nickname is already in use`.  
            Wait, numeric replies often include the "offending parameter" before the colon text:  
            Specifically for 433: it should be `<client> 433 <clientNick> <newNick> :Nickname is already in use`.  
            Actually, the format from irchelp:  
            [irchelp.org](https://www.irchelp.org/protocol/rfc/chapter6.html#:~:text=433%20%20%20%20,Nickname%20is%20already%20in%20use) shows `433 ERR_NICKNAMEINUSE "<nick> :Nickname is already in use"`.  
            So the numeric message includes the new nickname as the first param after the code, then colon text. Which implies:  
            `:<server> 433 <userNick> <attemptedNick> :Nickname is already in use`.  
            Our sendNumeric as defined will use u->nickname after code, then expects in params the attemptedNick and the message text separately? But we appended colon in last param.  
            If we pass params={"NewNick", ":Nickname is already in use"}, our code will output:  
            `:server 433 <userNick> NewNick :Nickname is already in use`.  
            That's correct as desired.  
            We just have to ensure to always put the colon in the last param we pass to sendNumeric so that function knows to treat it as trailing param.
            
    - For simpler replies like 331 RPL_NOTOPIC "<channel> :No topic is set":  
        sendNumeric(u, 331, {channel, ":No topic is set"});  
        That yields `:server 331 <userNick> #chan :No topic is set`.
        
    - For 353 RPL_NAMREPLY, format is "<symbol> <channel> :<names>", so call:  
        sendNumeric(u, 353, {"=", channel, namesWithPrefixes});  
        Here namesWithPrefixes string already contains the "@" etc and is prefixed with ":" or not?  
        Actually, if we want colon before the list, we should include it as part of the last param.  
        So better: sendNumeric(u, 353, {"=", channel, ":" + namesList});  
        That yields `:server 353 <userNick> = #chan :nick1 nick2`.  
        Perfect.
        
    - All right.
        
- **Edge case**: If user not registered and we call sendNumeric, our function currently will put u->nickname. That might be empty (if user hasn't set nick). Possibly some servers use "_" in that field before registration. We can handle:  
    if (u->nickname.empty()) targetName = "_"; else targetName = u->nickname;  
    Use that in sendNumeric prefix building.
    
- **No responses on NOTICE**: We implement in handlers by simply not calling sendNumeric. We might also incorporate a check inside sendNumeric if needed, but easier to manage in logic.
    
- **Completing welcome**:
    
    - In finishRegistration, after sending 001-004 and possibly others, we might also want to send LUSERS or MOTD:
        
        - If we have a short MOTD text or file, send 375 (start), 372 (lines), 376 (end).
            
        - Or send 422 ERR_NOMOTD if no MOTD configured.
            
        - If time, implement a basic MOTD:
            
            - store it in a config string or file loaded at startup.
                
            - If exists:  
                sendNumeric(u, 375, {":- " + serverName + " Message of the Day -"}); // header  
                split MOTD by lines, for each: sendNumeric(u, 372, {": " + line});  
                sendNumeric(u, 376, {":End of MOTD"});
                
            - If not: sendNumeric(u, 422, {":MOTD File is missing"}).
                
        - Also optional: send 251, 252, 253, 254, 255 (LUSERS info: number of users, ops, channels, etc).  
            Not necessary but nice. We can skip or do minimal:
            
            - 251: "<client> :There are <uCount> users and <iCount> invisible on <sCount> servers".
                
            - Because single server, ignore servers part: e.g. "There are X users on 1 server".
                
            - 255: "I have X clients and Y channels".
                
            - For brevity, skip or keep simple count.
                
- **Testing**:
    
    - Simulate a user connecting (NICK/USER), ensure 001-004 come properly formatted.
        
    - Simulate error conditions (like NICK in use) ensure it matches RFC format from references.
        
    - Simulate a user joining channel with no topic: check 331 etc.
        
    - Ensure trailing param formatting is correct (there should be exactly one colon in each output line preceding the last param's content).
        
    - Logging as needed to verify.
        

## Fase de Pruebas

### Pruebas Unitarias del Parser

- **Objetivo**: Validar que el parser separa y reconoce correctamente prefijo, comando y parámetros en diversos casos, y que maneja errores de formato adecuadamente. Estas pruebas no requieren una conexión real; se pueden llamar las funciones de parsing directamente con strings simuladas.
    
- **Casos de prueba para el _lexer/parser_**:
    
    1. **Mensaje completo con prefijo**: Input: `":Nick!user@host PRIVMSG #channel :Hola mundo\r\n"`. Verificar que:
        
        - `prefix == "Nick!user@host"`,
            
        - `command == "PRIVMSG"`,
            
        - `params == ["#channel", "Hola mundo"]`.
            
        - Confirmar que el espacio después de prefijo se consume y que el parámetro "Hola mundo" incluye el espacio internamente y no se corta.
            
    2. **Mensaje sin prefijo**: Input: `"PING :12345\r\n"`. Resultado esperado:
        
        - `prefix` vacío,
            
        - `command == "PING"`,
            
        - `params == ["12345"]` (un solo parámetro trailing).
            
        - Además, probar `"PING 12345\r\n"` (sin dos-puntos, aunque según RFC debería llevarlo porque puede considerarse parte del param): nuestro parser lo tomaría como `params=["12345"]` igualmente.
            
    3. **Comando numérico**: Input: `":server.example.com 001 NewUser :Welcome...\r\n"`. Debería parsearse como:
        
        - `prefix == "server.example.com"`,
            
        - `command == "001"` (lo mantiene como string, aunque podríamos convertir a int si quisiéramos),
            
        - `params == ["NewUser", "Welcome..."]`.
            
        - Confirmar que los dígitos del comando no se confundan con param.
            
    4. **Multiparámetros sin trailing**: Input: `"USER guest 0 * :Real Name\r\n"`. Esperado:
        
        - `command == "USER"`,
            
        - `params == ["guest", "0", "*", "Real Name"]` (Real Name como trailing, que contiene espacio).
            
        - Adicionalmente, `"USER guest 0 * :Real Name LastName\r\n"` debería unir "Real Name LastName" en un solo parámetro.
            
    5. **Líneas múltiples en buffer**: Simular que el buffer recibe `"PING :token\r\nPING :token2\r\n"` en un solo bloque. Verificar que el separador de líneas funciona:
        
        - Debe extraer primero "PING :token", luego en siguiente iteración "PING :token2". Cada uno parseable a command "PING", param trailing "token"/"token2".
            
        - Confirmar que el resto del buffer queda vacío al final.
            
    6. **Mensaje incompleto**: Simular que buffer recibe `"PRIV"` sin CRLF. El parser no debe producir un resultado aún (esperar). Luego simular llegada de rest `"MSG target :hi\r\n"` – concatenado se vuelve `PRIVMSG target :hi\r\n` – ahora debe emitir un mensaje parseado:
        
        - `command == "PRIVMSG"`, params `["target","hi"]`.
            
        - Esto prueba que el ensamblaje de fragmentos con CRLF funciona.
            
    7. **Errores sintácticos**:
        
        - Sólo prefijo sin comando: Input: `":nick!u@h \r\n"` (prefix seguidos de espacio y CRLF). Nuestro parser podría retornar command = "" o marcar error. Comprobar que detectamos la ausencia de comando. En contexto real, tras parsear, handleCommand vería msg.command vacío y podría invocar error desconocido.
            
            - En la prueba, podríamos adaptarlo: Ver que parser result indica falta.
                
        - Exceso de parámetros: Crear un mensaje con 16 parámetros: e.g. `"CMD a b c d e f g h i j k l m n o p q\r\n"` (16 params after CMD). Decidir esperado: quizás truncamos o error.
            
            - Suponer decidimos truncar a 15. Entonces parser debe retornar 15 params (the last one maybe aggregated or last is dropped). Confirmar se manejó (o if we choose error, ensure it flags an error state).
                
        - Parametro trailing mal ubicado: e.g. `"COMMAND middle1 :trailing middle2\r\n"`. Protocol forbids anything after a trailing param. Nuestro parser al ver ':' at "middle2" might treat it as trailing erroneously or, if coded robustly, once trailing is taken, it should ignore " middle2" as part of trailing text. Ideally, "middle2" should be part of the trailing param string (since everything after colon is trailing). So expected output:
            
            - params = ["middle1", "trailing middle2"].
                
            - Confirm that we indeed combine "middle2" with trailing param because of the logic "take rest of line as trailing". (This tests that parser correctly doesn't split after colon).
                
    8. **Case sensitivity**: Ensure parser does not alter case of command or params. E.g. input `"JOIN #Test\r\n"` yields command "JOIN", param "#Test" exactly, not lowercased. (Case normalization is done in logic, not parser). This test just ensures parser preserves input case.
        

### Pruebas Unitarias de Manejadores de Comandos

Estas pruebas verifican la lógica de cada comando aisladamente, usando instancias de `ServerState`, `User` y `Channel` simuladas en memoria. Se llamarán los métodos handleX directamente y se inspeccionarán los efectos en el estado y los mensajes en cola para envío (por ejemplo, examinando los buffers de salida simulados en los User).

- **Prepara un entorno de prueba**: Crear un `ServerState` con serverName "irc.test", y crear algunos objetos `User` representando clientes de prueba (p.ej. Alice, Bob). Podemos simular su conexión estableciendo su `nickname`, etc. Conectar esos usuarios a serverState.users map. También podemos crear un par de channels para contextos de pruebas de canal.
    

#### Pruebas de comandos de registro y usuario:

1. **NICK command**:
    
    - Caso éxito inicial: Crear User sin nick. Llamar `handleNick(user, {"Alice"})`. Verificar:
        
        - `user.nickname == "Alice"`,
            
        - `ServerState.users` contiene "alice" key mapping to user.
            
        - No welcome sent yet (porque USER no completado).
            
        - No errors in user's send buffer.
            
    - Nick en uso: Añadir otro User con nick "Bob". Para userAlice intentar cambiar a "Bob":
        
        - Llamar `handleNick(alice, {"Bob"})`. Debería:
            
            - Detectar "bob" ya en map,
                
            - Enviar `433 ERR_NICKNAMEINUSE` a alice. Ver en alice.sendBuffer algo como `433 * Bob :Nickname is already in use` (si alice no registrada, target might be "*"). Si ya estaba registr. target "Alice".
                
            - Confirmar que alice.nickname no cambió (sigue "Alice").
                
            - Confirmar no duplicate insertion in map.
                
    - Nick inválido: e.g. `handleNick(alice, {"Inv@lid"})`. Esperado:
        
        - Enviar `432 ERR_ERRONEUSNICKNAME Inv@lid :Erroneus nickname`.
            
        - Nick no cambia, still old "Alice".
            
        - Map unchanged.
            
    - Cambio de nick post-registro: Tener userAlice in map with nick "Alice". Another free nick "Charlie".
        
        - Call `handleNick(alice, {"Charlie"})`.
            
            - Should remove "alice" from map, insert "charlie".
                
            - userAlice.nickname updated to "Charlie".
                
            - Broadcast: If alice was in channels with others, those others get NICK message. Simulate:
                
                - Put alice and bob in a Channel #test. Mark alice and bob in channel members.
                    
                - Bob has an output buffer. After handleNick, check Bob's buffer for `:Alice!user@host NICK :Charlie`.
                    
            - Check Alice's own buffer (some servers don't echo back to origin on nick change because the client already knows, but since we broadcast to all members including them, Alice might also get it. Actually, the convention: the user sees their own nick change response as if from server: no separate numeric, but the client might see nothing if we don't send to them. Many servers do send the user their own NICK message for consistency).  
                We likely implemented broadcast to all members including self. So check Alice buffer as well.
                
    - Registro completo: Create new user with no nick or user set:
        
        - handleNick -> sets nick. handleUser -> sets rest and triggers welcome.
            
        - Test: userDave with nickname "Dave", call handleNick then handleUser:
            
            - After handleUser, check userDave.sendBuffer for 001,002,... welcome messages.
                
            - Confirm 001 line includes "Welcome... Dave!user@host".
                
            - Confirm isRegistered true.
                
        - Also test inverse order: handleUser then handleNick:
            
            - After handleNick second, should trigger welcome similarly.
                
            - No double-welcome etc.
                
    - Already registered sending USER again: Simulate user with isRegistered true. Call handleUser on them:
        
        - Expect ERR_ALREADYREGISTRED 462 in buffer.
            
        - No change to user fields.
            
2. **QUIT command**:
    
    - Setup: userAlice and userBob in server, both in channel #test. Possibly userCharlie in another channel or not.
        
    - Call `handleQuit(alice, {":Gone"})`. Validate:
        
        - alice should be removed from serverState.users.
            
        - alice's channels list is empty (she left #test).
            
        - #test channel members no longer contain alice.
            
        - If #test had only Alice and Bob, now only Bob or channel removed if Bob also parted? Actually if Alice and Bob were there, after Alice leaves, Bob remains, channel stays.
            
        - Bob's buffer should contain `:Alice!user@host QUIT :Gone`.
            
        - Charlie (not in same channel) should not receive anything.
            
        - The QUIT message goes to any user that shared at least one channel:
            
            - If we had multiple channels, ensure recipients list is union (no duplicates).
                
            - If Bob and Alice shared two channels, Bob still should get one QUIT.
                
        - Ensure even if channel is empty and removed as result of quit, the part was handled and others got the quit (if any others).
            
        - Check that after quit, if we attempt to send something to Alice (like further commands or ping from server), no such user exists. Possibly ensure user object is freed (if using raw pointers).
            
    - Also simulate a user quitting that is on multiple channels:
        
        - userAlice on #chan1 and #chan2 with some other people. Make sure those others all get QUIT.
            
        - If user was sole member in one channel (#chan2), that channel is removed.
            
    - Ensure no numeric replies were sent to quitting user (server doesn't send anything to confirm quit).
        
    - If we log or print something on quit, can check logs for a message.
        

#### Pruebas de comandos de canal:

3. **JOIN command**:
    
    - **Join new channel**: userAlice sends `JOIN #newchan`.
        
        - Expected:
            
            - Channel created with name "#newchan" in serverState.channels.
                
            - Alice is in channel members and is op (check chan->isOperator(alice) true).
                
            - Alice's channel list contains newchan.
                
            - Alice's buffer:
                
                - should have no explicit "JOIN" message from server (some servers echo the JOIN back to the joiner, others rely on client to assume join? Actually, as per spec, server sends JOIN to all channel members, which includes the joiner, so yes, Alice gets her own JOIN echo).
                    
                - Then RPL_TOPIC (331 since no topic) and RPL_NAMREPLY & RPL_ENDOFNAMES for #newchan.
                    
                - Check the format of NAMES: " = #newchan :@Alice". (Alice as @).
                    
                - Check RPL_ENDOFNAMES comes after.
                    
            - No other user to notify since she was alone.
                
    - **Join existing channel**: Create channel #test with Bob already inside.
        
        - Bob is op.
            
        - Alice executes JOIN #test.
            
        - Validate:
            
            - Alice added to #test members.
                
            - Her op status: since Bob was already there, new user is not op by default.
                
            - Both Alice and Bob should get messages:
                
                - Bob's buffer: `:Alice!user@host JOIN #test`.
                    
                - Alice's buffer: `:Alice!user@host JOIN #test` (the echo) plus topic or not, and NAMES list:
                    
                    - If Bob had set a topic "Hello", Alice should get RPL_TOPIC with it.
                        
                    - If no topic, 331.
                        
                    - Then NAMES showing Bob (maybe "@Bob") and Alice (no prefix if not op/voice).
                        
                - Bob might get an update to NAMES? Actually, server typically does not send NAMES to existing members on a new join, they just see the JOIN. So Bob will not get a NAMES list or anything (client can request /names if needed).
                    
            - Channel state: still exists, now two members.
                
            - Alice's channel list updated.
                
            - Invite list: if Alice was on invite list and channel is +i, ensure if that scenario we remove invite after join (test an invite scenario: mark channel +i and invite Alice, then do join).
                
    - **Join with key**:
        
        - Create channel #keychan with key "secret", and +k mode. Bob is inside and channel has that key.
            
        - Alice tries `JOIN #keychan` without key -> should get ERR_BADCHANNELKEY.
            
            - Alice buffer: "475 #keychan :Cannot join channel (+k)".
                
            - Not joined, not in channel.
                
        - Alice tries with correct key: `JOIN #keychan secret`:
            
            - Should succeed (assuming other conditions fine).
                
            - Validate as existing channel join above, with no error.
                
        - Alice tries with wrong key: ERR_BADCHANNELKEY again.
            
        - Possibly test channel with multiple keys (not needed since one channel at a time).
            
    - **Invite-only channel**:
        
        - Channel #invitechan set +i, Bob inside as op.
            
        - Alice tries join without invite: ERR_INVITEONLYCHAN.
            
        - Bob invites Alice using handleInvite (populating inviteList).
            
        - Alice tries join again, this time allowed:
            
            - join successful and remove invite from inviteList.
                
            - Standard join notifications.
                
    - **Channel full (+l)**:
        
        - Channel +l limit = 1, Bob is inside.
            
        - Alice join -> ERR_CHANNELISFULL 471.
            
        - If Bob leaves and tries to join and others, etc.
            
        - Remove or adjust limit for further test if needed.
            
    - **Banned user**:
        
        - Channel where ban list has Alice's mask (simulate user mask).
            
        - Alice tries join -> ERR_BANNEDFROMCHAN 474.
            
        - If an exception mask that matches Alice is set, then allow join despite ban:
            
            - Add an exception for Alice's host, then join should succeed.
                
    - **Max channels per user**:
        
        - Set ServerState.maxChannelsPerUser = 1 for test.
            
        - Alice already in #chan1. Then tries to join #chan2:
            
            - Should get ERR_TOOMANYCHANNELS 405.
                
            - Channel #chan2 should not be created if it didn't exist.
                
            - Alice remains only in #chan1.
                
        - Could test when at limit and they PART one, then can join new again.
            
    - **Multiple channels in one JOIN**:
        
        - E.g. `JOIN #chanA,#chanB keyA,keyB`.
            
        - If implemented, test:
            
            - Both channels get joined appropriately.
                
            - If one fails (like #chanB has wrong key), it returns error for that and still joins #chanA.
                
            - Confirm responses: ideally send errors for B and normal join sequence for A.
                
            - This is more integration heavy, but if implemented, ensure splitting logic correct.
                
    - After each join, verify internal data (channel members, maps) and output messages thoroughly.
        
4. **PART command**:
    
    - Basic case: Alice and Bob in #test.
        
        - Alice does `PART #test :Bye`.
            
        - Expect:
            
            - Alice removed from #test members.
                
            - Channel still exists with Bob if Bob remains (he might be sole now).
                
            - Bob's buffer: `:Alice!user@host PART #test :Bye`.
                
            - Alice's buffer: `:Alice!user@host PART #test :Bye` (the echo to herself).  
                (If we included self in broadcast).
                
            - Check Alice's channel list no longer has #test.
                
            - If #test had topic, it remains but irrelevant to test.
                
    - No message parameter:
        
        - Alice parts with no trailing param (just "PART #test").
            
        - Then Bob sees `... PART #test` with no " :" message part.
            
        - Our implementation might send a colon but empty text (which appears as "PART #test :"), or we could omit colon. It's tricky because sendNumeric auto-colon last param but sendMessage in broadcast we control fully:  
            likely if message was "", we formed "PART #test" without trailing segment. That is valid.
            
        - Ensure format is correct (we might ensure no double colon etc.)
            
    - Part channel not in:
        
        - Alice calls PART on #notamember:
            
            - If channel exists but she not in it -> ERR_NOTONCHANNEL.
                
            - If channel doesn't exist -> ERR_NOSUCHCHANNEL.
                
            - Confirm those errors in buffer.
                
            - No change in state.
                
    - Multi-channel part:
        
        - If implemented "PART #chan1,#chan2 :Later".
            
            - Should part both, and send separate PART for each to respective channel members (with same reason).
                
            - If one channel fails, error for that but still attempt next.
                
            - Test both success: see both parted properly.
                
            - Possibly test a mix: part one valid, one invalid.
                
    - Part causing channel destruction:
        
        - If Alice and Bob are only in #test, and Alice parts:
            
            - Bob remains, channel not destroyed.
                
            
            - If Bob then parts too (channel becomes empty), after Bob part, we should delete channel.
                
            - That deletion might not produce any broadcast because no one left to inform, but the event should remove from map.
                
            
            - Test scenario: single user channel:
                
                - Alice alone in #solo.
                    
                - She PARTs #solo.
                    
                - Channel should be removed from map.
                    
                - Alice gets PART echo.
                    
                - No one else to notify.
                    
                - Next time someone tries to join #solo, it's as new channel.
                    
    - Check that if a user is in multiple channels and parts one, it doesn't affect others.
        
5. **MODE command (channel modes)**:
    
    - Prepare channel #test with Alice (op) and Bob (non-op).
        
    - **Query mode**: Bob (non-op) sends `MODE #test`:
        
        - Should get RPL_CHANNELMODEIS:
            
            - e.g. if no modes set, `:server 324 Bob #test +`.  
                Actually, if no flags set, maybe just "+" or an empty string? Usually it might show just "+" (some impl do that).  
                Could just show `:server 324 Bob #test :` with nothing? Or "+nt" by default if those are default?  
                We'll see what we decided (if we defaulted to no ext msg and topic lock).  
                Suppose we default to +nt on channel creation:  
                Then expecting `324 Bob #test +nt`.  
                Possibly also `329` with creation time.
                
        - Check Bob receives it even though not op (he can query).
            
    - **Give/take operator**:
        
        - Alice as op issues `MODE #test +o Bob`.
            
            - Bob should now be marked op in channel.
                
            - Broadcast to channel: `:Alice!u@h MODE #test +o Bob`.
                
            - Bob's membership status updated.
                
            - If Bob's user object had any flag or not (we track per channel in Channel structure, not global).
                
            - Bob (target) gets that message too so his UI can show he's op.
                
        - Then Alice issues `MODE #test -o Bob`:
            
            - Bob removed op.
                
            - Broadcast `... -o Bob`.
                
            - Confirm changes.
                
        - If Alice tries to +o someone not in channel (Charlie):
            
            - Should get ERR_USERNOTINCHANNEL.
                
            - No mode change done.
                
        - If Bob (non-op) tries `MODE #test +o himself`:
            
            - Should get ERR_CHANOPRIVSNEEDED (because Bob isn't op to give op).
                
            - No effect.
                
    - **Give/take voice**:
        
        - Test similarly with +v and -v.
            
        - Only difference: voiced users can talk under +m but not op.
            
        - Confirm broadcast and internal state voiceSet updated.
            
    - **Toggle simple flags** (i, t, n, m, p, s, r):
        
        - Alice sets +m moderated:
            
            - chan->moderated becomes true.
                
            - Broadcast `+m` to channel.
                
            - If we grouped with others or just alone, likely just `+m`.
                
            - Bob receives it, as does Alice.
                
        - Alice sets -m:
            
            - back to false, broadcast.
                
        - Try set +p and +s together:
            
            - If our logic forbids both simultaneously:
                
                - If channel already +p and Alice tries +s:  
                    We might either override +p by setting +s and removing +p automatically (some ircds do that since they can't coexist).  
                    Or we check and prevent one if the other is on:  
                    Perhaps better to not allow both:  
                    e.g. if tries "+ps":
                    
                    - We could set +s and drop +p, or vice versa.
                        
                    - Since RFC says MUST NOT both be set[datatracker.ietf.org](https://datatracker.ietf.org/doc/html/rfc2811#:~:text=RFC%202811%20%20%20,April%202000), maybe choose one.
                        
                    - We didn't explicitly implement a check for that in design (maybe in semantic validations).  
                        Possibly we ignore here and add in semantic tests that server should not allow both (like if user tries +ps, only +s is applied and we skip +p or apply then remove).  
                        For test, if we left it unconstrained:
                        
                    - Then the channel might end up with both flags, violating RFC.
                        
                    - Our optional validation might address that, e.g. automatic removal of the other or disallow.
                        
                
                - Let's say we implement: if one is set, un-set the other.  
                    Test that setting +s on a +p channel results in channel being +s -p:
                    
                    - Possibly output should show "+s -p" combined if done in one command.  
                        If user typed "MODE #chan +s":  
                        We could auto remove +p silently (or also output it).  
                        Possibly just set +s and internally remove p without telling (though that breaks rule quietly).  
                        Better to reflect: output mode change `+s -p`.  
                        However, might be too detail. If not implemented, we skip this test or consider it in semantic tests.
                        
        - All other toggles similar, just ensure broadcast and internal state.
            
        - If a non-op attempts any of these toggles:
            
            - They all require op (except maybe -o on themselves but typically still not allowed).
                
            - So Bob trying `MODE #test +m` yields ERR_CHANOPRIVSNEEDED.
                
    - **Key mode**:
        
        - Alice sets +k with param:
            
            - Provide "key123".
                
            - Channel.key set, hasKey true.
                
            - Broadcast `+k key123` to channel (though IRL, the key is usually hidden in server replies? Actually, when an op sets a key, the key is visible to all in that channel in the MODE message. It's not secret since channel members can see it. But it is secret to those outside as they wouldn't see the MODE).
                
            - Test Bob receives `MODE #test +k key123`.
                
            - Now channel requires key to join.
                
        - Alice tries to set a new key while one is already set:
            
            - Some servers respond ERR_KEYSET (467) "Channel key already set". We didn't mention that, but maybe we implement: if hasKey true and adding +k:  
                we might either override the key (like changing password) or error.
                
            - Possibly allow change (changing key is allowed for ops usually). So we should allow setting a new key (update stored key).
                
            - If we allowed, broadcast will just show new key.
                
            - Test that scenario: initially key "key123", now Alice `MODE #test +k newkey`.  
                Check channel.key updated, broadcast +k newkey.
                
            - Then test removing key:
                
                - Alice `MODE #test -k`.
                    
                - Channel.hasKey false, key cleared.
                    
                - Broadcast "-k" (no param).
                    
                - Bob sees `MODE #test -k` (meaning channel unlocked).
                    
        - If a non-op tries to remove a key (which might be considered lesser action?), still requires op. So Bob -k fails with 482.
            
    - **Limit mode (+l)**:
        
        - Alice sets +l 5:
            
            - Channel.userLimit set 5.
                
            - broadcast "+l 5".
                
        - If she sets +l 0 or -l:
            
            - Channel.limitActive false (we might unify to hasLimit flag).
                
            - broadcast "-l".
                
        - If tries +l with no param:
            
            - Should error 461.
                
        - Non-op tries: error 482.
            
    - **Ban/exception/invite lists**:
        
        - Initially no bans:
            
            - Alice `MODE #test +b` with no param:
                
                - Should list ban list (which is empty).
                    
                - Expect: RPL_BANLIST none, then RPL_ENDOFBANLIST.
                    
                - Actually, if empty, likely just RPL_ENDOFBANLIST.
                    
                - Test she gets that numeric.
                    
            - Alice sets +b _!_@somehost:
                
                - Channel.banList has that mask.
                    
                - broadcast "+b _!_@somehost".
                    
                - Bob sees it.
                    
                - Alice sees it (they both get mode broadcast).
                    
                - After, test list: `MODE #test +b` now returns one entry:
                    
                    - RPL_BANLIST <channel> <mask> [who time]. We didn't implement who/time, but maybe we can include the setter nick and current time? If not, just send mask in place of both as minimal:  
                        sendNumeric(u, 367, {channel, mask});  
                        Actually RFC requires "setter time" but if not stored, some servers send the mask with no additional info.  
                        Could skip those fields.
                        
                - Then RPL_ENDOFBANLIST.
                    
                - Check output lines.
                    
            - Alice sets -b _!_@somehost:
                
                - Removes from banList.
                    
                - broadcast "-b _!_@somehost".
                    
                - List now empty again (if list requested).
                    
            - Try removing a ban not present:
                
                - Our logic would still broadcast -b with the given mask even if it wasn't found? Or we may do nothing. We attempted to find and remove, if not found we still broadcast mode? Possibly should not broadcast if nothing changed. But our code might still output mode change if user gave it.
                    
                - Decide: ideally do nothing. If we implemented find, then if not found we might skip adding to outputModes, so no broadcast.
                    
                - So test: remove nonexistent ban yields no mode message. Possibly could send no error either.
                    
            - Similarly for +e and +I lists:
                
                - They behave like ban list with different numeric codes.
                    
                - We can test adding an exception, listing it, removing it.
                    
                - The differences: invite list +I has similar list codes.
                    
        - Non-operator tries to list or set bans:
            
            - Should get 482 (can't view or change lists if not op in many IRCd). But some allow listing even if not op? Not sure.
                
            - We'll require op. So Bob `MODE #test +b` -> 482.
                
            - Bob `MODE #test +b mask` -> 482.
                
    - **User modes**:
        
        - By design, we allowed user to set +i (invisible) on themselves.
            
        - Test: Bob (not oper) sends `MODE Bob +i`:
            
            - Should succeed: Bob.invisible = true.
                
            - Possibly send Bob a confirmation:
                
                - either a numeric 221 showing new modes or an echo `:Bob!user@host MODE Bob :+i`.
                    
                - We chose numeric.
                    
                - Check Bob's buffer for "221 Bob +i".
                    
            - Others should not see anything (user mode changes are not broadcast).
                
        - Bob tries to set +o on himself:
            
            - Should get ERR_NOPRIVILEGES or ERR_UMODEUNKNOWNFLAG (depending what we decided).
                
            - We used 481 for that presumably.
                
            - Buffer: "481 Bob :Permission Denied- You're not an IRC operator" (if we gave that text).
                
            - Bob remains not IRCop.
                
        - Bob tries to set an invalid user mode letter, like +z:
            
            - ERR_UMODEUNKNOWNFLAG 501.
                
        - Oper privileges:
            
            - If we had an Oper command to grant +o, then test that (if implemented).
                
            - We likely skip actual OPER command test since not requested, but could simulate making a user an IRC operator by setting user.isIRCOperator = true in state manually and then testing they can see some differences (like maybe override channel +i? But we didn't implement that).
                
            - Skip due to complexity.
                
    - **Complex mode string**:
        
        - Test multiple changes in one command:
            
            - e.g. Alice does `MODE #test +im -n`.  
                This means +i +m and -n combined.
                
                - If channel was initially -i -m +n:  
                    After: +i sets inviteOnly, +m sets moderated, -n removes noExt.
                    
                    - Combined output ideally: `+im-n` (with no param needed).
                        
                    - Order in output could be exactly as input order or consolidated, depending how implemented. Our approach likely preserves order and sign group separation properly.
                        
                    - Check channel flags all updated accordingly.
                        
                    - Confirm broadcast string and resulting state:  
                        Channel should now have inviteOnly and moderated true, noExt false.
                        
                
                - We ensure our code handled the change of sign in the middle.
                    
            - Another: `MODE #test +ov Bob Charlie` (giving op to Bob and voice to Charlie in one go).
                
                - This should result in:
                    
                    - Bob gets op, Charlie gets voice.
                        
                    - Broadcast: `+ov Bob Charlie`.
                        
                    - If Charlie or Bob not in channel, error should cut it short or skip accordingly.
                        
                    - Test scenario:
                        
                        - Charlie in channel but not op or voice currently.
                            
                        - Alice (op) sends +ov Bob Charlie:  
                            ~ Even though Bob is already op perhaps, if he is not, then fine.  
                            ~ If Bob was not in channel or Charlie not, error should have occurred for that portion:  
                            test: Charlie is in channel, Bob is not in channel.  
                            Then when processing 'o Bob', we error 441 and likely stop or skip further processing of the mode string depending on how we wrote it (we might break out of loop on error or continue).  
                            If we continued, next tries give voice to Charlie which would succeed.  
                            But often, encountering an error stops processing subsequent modes in same command.  
                            Did we plan that? Not explicitly, but could be prudent: If sendNumeric called, perhaps break out.  
                            Let's see design: in mode processing we had "break out (stop loop) on some errors like param missing or listing".  
                            But for usernotinchannel, we didn't specify break out. Possibly just skipping that mode and continuing might be okay.  
                            It's a design choice.  
                            ~ If we skip just Bob, then Charlie gets voice and we output `+v Charlie` (since Bob part failed and we might exclude it from output).  
                            Bob also got an error numeric individually.  
                            ~ We should test which way we did.
                            
                        - If we break on error, then voice won't be set.
                            
                        - Possibly better UX to skip failed mode and process rest.
                            
                        - Our pseudo code suggests we might continue after 441 (since we didn't break in that case).
                            
                    - So expected:
                        
                        - Bob not in channel: Alice receives ERR_USERNOTINCHANNEL Bob.
                            
                        - Charlie gets voice and channel broadcast just `+v Charlie` (since Bob's +o was dropped).
                            
                        - If implemented differently, we adjust expectation.
                            
            - Param required but not given mid-sequence:
                
                - e.g. `MODE #test +o+v Bob` (which is unusual syntax but say user forgot param for +v).
                    
                - After +o Bob done, sees +v expects param but not present, sends ERR_NEEDMOREPARAMS and stops.
                    
                - Bob got op but voice not given to anyone.
                    
                - Check that no broadcast or partial output occurred (we might have done +o Bob already and queued it to outputModes, then discovered missing param for +v and broke out. Possibly we would still broadcast +o Bob).
                    
                - Should we? Arguably yes, because +o was valid and executed. Many IRCDs would apply what came before error and then stop at error (the state is changed and user gets error for the rest).
                    
                - So Bob would become op and broadcast +o Bob, then error about need more params for +v.
                    
                - That might double surprise user (they see mode change succeeded but also error).
                    
                - But it is how it could behave.
                    
                - We can test if our code did accumulate and flush partially or dropped all on error.
                    
                - According to our design, we break out of loop on missing param, but we likely still broadcast what was gathered before break.
                    
                - So test scenario:
                    
                    - Channel with Bob not op.
                        
                    - Alice sends `MODE #test +ov` (no param for v).
                        
                    - Expect:  
                        Bob gets op (channel changed).  
                        Alice gets ERR_NEEDMOREPARAMS for MODE.  
                        The broadcast for +o Bob could either have been sent or not.  
                        Possibly yes if we appended before encountering error.
                        
                    - Check results: Bob is op now definitely.
                        
                    - If broadcast was suppressed due to error, Bob wouldn't know he is op except his client might treat error differently. But since state changed, he should know.
                        
                    - It's a corner case. We'll see how to handle (maybe best to still send +o).
                        
    - Confirm state after a series of operations and that output messages match expected format and content from RFC where possible (like numeric texts).
        
    - Also test listing modes:
        
        - e.g. RPL_CHANNELMODEIS includes key and limit if set:
            
            - Set a key and limit, then query mode:
                
            - Ensure 324 includes those (e.g. "+kl 5 keypass").
                
        - Listing ban list with one entry:
            
            - We should see `367 <channel> <mask> <setter> <timestamp>` ideally.
                
            - If we didn't implement setter/time, maybe we only send mask:
                
                - Possibly we leave <setter> blank or put the server or channel name.
                    
                - Might test for presence of some placeholder; if none, just ensure the mask appears.
                    
                - We can adapt expectation to what we implemented.
                    
            - End list numeric present.
                

#### Pruebas de comandos de tópico, kick, invite, names, privmsg:

6. **TOPIC command**:
    
    - Create #topicchan with Alice and Bob.
        
    - No topic initially:
        
        - Bob (not op) does `TOPIC #topicchan`:
            
            - Should receive RPL_NOTOPIC (331).
                
        - Bob tries `TOPIC #topicchan :NewTopic` (attempt to set while not op and channel default +t?):
            
            - If channel has +t by default (if we set that), Bob gets ERR_CHANOPRIVSNEEDED.
                
            - If channel -t (anyone can set), he could set it:
                
                - But by default we might have +t if we followed typical default.
                    
                - Let's explicitly remove +t for testing:  
                    channel.topicOpOnly = false.
                    
                - Then Bob sets topic:  
                    = Channel.topic updated to "NewTopic".  
                    = Broadcast `:Bob!u@h TOPIC #topicchan :NewTopic` to all (Alice sees it, Bob sees it).
                    
                - Confirm topic stored.
                    
                - Then Alice do `TOPIC #topicchan` to query:  
                    -> RPL_TOPIC (332) with "NewTopic".  
                    -> Possibly RPL_TOPICWHOTIME (333) if implemented (we might skip or test if skip).
                    
        - Now test with +t:
            
            - Set channel.topicOpOnly true (like an op did +t).
                
            - Bob tries to change again:
                
                - Should get ERR_CHANOPRIVSNEEDED now.
                    
                - Topic remains "NewTopic".
                    
            - Alice (op) changes topic:
                
                - `TOPIC #topicchan :AnotherTopic`.
                    
                - Should succeed:  
                    = Channel.topic = "AnotherTopic".  
                    = Broadcast to both: `:Alice!.... TOPIC #topicchan :AnotherTopic`.
                    
                - Query yields 332 with updated text.
                    
                - Possibly check that setting topic to empty:
                    
                    - `TOPIC #topicchan :` (Alice clears topic).
                        
                    - Channel.topic becomes "" (no topic).
                        
                    - Broadcast `TOPIC #topicchan :` (some servers might not send any text after colon).
                        
                    - Bob sees channel now has no topic.
                        
                    - Query yields 331 No topic.
                        
    - If channel secret and someone outside tries to get topic:
        
        - Not easily tested here without building a separate user not in channel. We can simulate:
            
            - Charlie not in channel does `TOPIC #topicchan`.
                
            - If channel +s or +p:  
                expect ERR_NOTONCHANNEL (we decided to hide).  
                If we allowed query for public, that we tested with Bob (though Bob was in).  
                We could remove Charlie to test:
                
                - Without +s, a user not in channel queries might still get topic (some networks allow, some require membership).
                    
                - Our design said if not in and not secret, maybe allow.
                    
                - If implemented, test:  
                    ~ Remove any secret flag, have Charlie query:  
                    ~ Expect RPL_TOPIC with actual topic.  
                    ~ Or if we required membership regardless, he would get ERR_NOTONCHANNEL.
                    
                - Decide likely more correct: if channel is not secret, allow reading topic from outside (like /list often shows topics).  
                    ~ So test that scenario if we supported.
                    
    - If multiple channels in TOPIC param (protocol allows only one channel at a time, I think, so we skip that scenario).
        
7. **KICK command**:
    
    - Setup: #kickchan with Alice (op) and Bob (member). Charlie not in channel.
        
    - Alice kicks Bob:
        
        - Call handleKick(Alice, {"#kickchan", "Bob", "Goodbye"}).
            
        - Validate:
            
            - Bob is removed from #kickchan.
                
            - Channel still exists with only Alice.
                
            - Alice's buffer: should she get a kick message? Actually, yes, server will send KICK to all in channel _including the kicker_ (the kicker sees the message as confirmation).
                
            - Bob's buffer: `:Alice!u@h KICK #kickchan Bob :Goodbye` (the actual notification he's kicked).
                
            - Alice's buffer: the same KICK message (to see that she did it, possibly).
                
            - Bob's user channel list no longer includes #kickchan.
                
            - Channel members list now only Alice.
                
    - Kick with no reason:
        
        - Alice kicks Bob again (or another scenario) with no comment param.
            
        - Then Bob sees `KICK #chan Bob :Alice` if we used default reason as kicker's nick or something.
            
        - If we left it empty:
            
            - Possibly `KICK #chan Bob` with no colon part (not allowed by protocol, must have a colon for comment because comment parameter is required by syntax but can be empty string).
                
            - We might fill with default to ensure the presence of trailing param.
                
            - Check what we did: If comment empty, some servers use nick as default or just nothing after colon.
                
            - If we did nothing special, our send might have omitted " :reason".
                
            - That could be considered an invalid message format (though colon could be omitted if last param is empty? Actually, trailing param can be empty after colon, meaning the colon is present but nothing after).
                
            - Best to include colon and nothing after. We should test what we implemented.
                
        - If default reason is implemented (like nick):
            
            - Bob sees ":Alice KICK #chan Bob :Alice".
                
            - It's a bit odd but known default on some servers.
                
        - Either way, ensure Bob's buffer gets a KICK line.
            
    - Non-op tries to kick:
        
        - Bob tries to kick Alice:
            
            - Expect ERR_CHANOPRIVSNEEDED (482).
                
            - No one removed.
                
            - No KICK message broadcast.
                
        - Kick target not in channel:
            
            - Alice tries `KICK #kickchan Charlie`:
                
                - Charlie not present, expect ERR_USERNOTINCHANNEL (441).
                    
                - No changes.
                    
        - Kick on unknown channel:
            
            - Alice `KICK #unknown nick` -> ERR_NOSUCHCHANNEL (403).
                
        - Multiple targets:
            
            - If we allowed "KICK #chan user1,user2",  
                test that user1 and user2 removed with one command:  
                might not have implemented, likely not. So skip.
                
8. **INVITE command**:
    
    - Setup: #invchan with Alice (op) and channel +i or not.
        
    - Basic success:
        
        - Alice invites Bob (who is offline or just not in channel).
            
        - handleInvite(Alice, {"Bob", "#invchan"}).
            
        - Check:
            
            - Alice's buffer: `341 Alice #invchan Bob` (or "Bob #invchan"? Actually numeric 341: "<channel> <nick>" per RFC).  
                Wait, RFC order:  
                According IRCHelp, `341 RPL_INVITING "<channel> <nick>"`[modern.ircdocs.horse](https://modern.ircdocs.horse/#:~:text=Numeric%20Replies%3A).  
                So likely our sendNumeric(341, {channel, nick}) yields `:server 341 Alice #invchan Bob`.  
                Good.
                
            - Bob's buffer: `:Alice!user@host INVITE Bob :#invchan`.  
                Note Bob sees the invite with channel after colon.
                
            - Channel.inviteList contains "Bob".
                
        - If Bob then joins:
            
            - Will be allowed due to invite, which we test in join tests (we did).
                
        - If channel was +i:
            
            - We already have needed scenario. If it was not +i, any member can invite but it's optional.
                
            - If we implement that only op can invite if +i, that's fine.
                
            - If not +i, we allowed any to invite as per assumption. Test:
                
                - Mark channel inviteOnly false.
                    
                - Bob invites Charlie (Bob is just member not op):
                    
                - It should succeed if channel not +i.
                    
                - Actually, did we restrict invite only if +i? Yes, we said if +i and not op then error, otherwise if not +i, any member can invite.
                    
                - So test Bob inviting Charlie:  
                    = Bob's buffer gets 341 confirmation.  
                    = Charlie gets invite message from Bob.  
                    = inviteList updated.
                    
            - Non-member tries to invite:
                
                - Charlie (not in channel) invites someone:
                    
                - Expect ERR_NOTONCHANNEL.
                    
                - No invite sent.
                    
    - Already on channel:
        
        - Alice tries to invite Bob who is already in #invchan:
            
            - ERR_USERONCHANNEL (443).
                
            - No invite list change or message.
                
    - Invite nonexistent nick:
        
        - Alice invites "NonexistentNick":
            
            - ERR_NOSUCHNICK (401).
                
            - No invite sent.
                
    - If multiple invites in one command not supported by IRC (invite only takes one nick at a time).
        
    - After Bob invited by Alice, check that Bob's user object or some state doesn't erroneously mark something; we only track invites in channel struct.
        
    - Perhaps test that after an invite, and Bob joins, the inviteList clears for Bob:
        
        - After Bob join, inviteList either cleared or at least Bob removed.
            
        - Possibly we do that already on join.
            
        - Test:
            
            - After Bob joins, check inviteList no longer contains "Bob".
                
        - If Bob didn't join but invite exists, and time passes, maybe we might not expire it. (Out of scope to test expiration unless we implemented).
            
    - If channel is +I (we didn't implement separate, but invite mask concept):
        
        - we handle via inviteList essentially as +I list in concept.
            
        - That aside, fine.
            
9. **NAMES command**:
    
    - Setup: channel with Alice (op), Bob, Charlie (secret?), etc.
        
    - Test 1: No param (all channels):
        
        - Suppose we have:
            
            - #chan1: Alice, Bob
                
            - #chan2: Bob, Charlie (and +s secret).
                
            - #chan3: Charlie (private +p).
                
            - User Dave calls NAMES with no param:
                
                - He is on none of those channels (Dave is separate).
                    
                - He should get names for #chan1 (because it's public and he can see it, with its members listed).
                    
                - Not for #chan2 (secret, omit entirely).
                    
                - Not for #chan3 (private, omit).
                    
                - Possibly a list of users not on any channel (not sure if we did that).
                    
                - Then end of list.
                    
                
                - Check Dave buffer:
                    
                    - line for #chan1: `353 * = #chan1 :@Alice Bob` (or target his nick or "*" depending on how we format when no param).
                        
                    - We might have chosen to send target as the user itself always, so "353 Dave = #chan1 :Alice Bob" is more likely (with @ prefix if op etc).
                        
                    - If we followed RFC to letter: when no channels param, they sometimes use the target as "*" in numeric replies.
                        
                    - But simpler: always use user nick as target param.
                        
                    - Acceptable.
                        
                    - Confirm Alice has @ if op etc.
                        
                
                - No lines for #chan2, #chan3 (skipped).
                    
                - End of names: `366 Dave * :End of NAMES list` or `366 Dave #chan1 :End of NAMES list`? Possibly when no param they often send 366 with "_" to close, or one per channel.  
                    We planned one global with "_" maybe.  
                    Check what we implemented.  
                    Possibly easier: we might have coded to send 366 per channel in loop, but if we skip channel, did we send something? We probably skip entire channel.  
                    Could do one at end:  
                    We'll adapt expectation accordingly.
                    
    - Test 2: One channel param:
        
        - Dave does `NAMES #chan1`.
            
            - Should get `353 ... = #chan1 :Alice Bob` (with proper prefixes).
                
            - Then `366 ... #chan1 :End of NAMES list`.
                
        - Dave does `NAMES #chan2` (secret, not in it):
            
            - If secret: likely no 353, just `366 #chan2 :End of NAMES list` (to indicate end of list with nothing).
                
            - Or possibly `401 No such nick/channel`? But for NAMES usually not, they just treat like empty.
                
            - We should have done the omit approach:
                
                - So Dave might get 366 or maybe nothing at all.
                    
                - But since we want to end each query, probably `366 #chan2 :End of NAMES list`.
                    
        - Bob does `NAMES` with param multiple:
            
            - e.g. `NAMES #chan1,#chan2`.
                
            - He is in #chan1 but not in #chan2.
                
            - For #chan1 (he's member): list everyone, including possibly himself.
                
            - For #chan2 (if secret and he's not in #chan2, skip listing).
                
            - He gets:
                
                - 353 for #chan1 including names.
                    
                - 366 for #chan1.
                    
                - Possibly 366 for #chan2 with no preceding 353 (just to indicate done).
                    
                - Check output.
                    
        - If user is in a channel, even if secret, they should get its names of course.
            
            - Eg. Charlie in #chan2 secret, Charlie `NAMES #chan2`:
                
                - He gets listing of #chan2 since he's in it, secret doesn't matter for him.
                    
                - If no param, he would also see it because he's in it.
                    
        - If channel has no users (rare scenario because channel removed if empty, unless safe channel concept):
            
            - Not testable as channel removed at empty.
                
        - If two channels share some users, no difference for listing.
            
    - Check formatting:
        
        - "353 <nick> <symbol> <channel> :names"
            
        - <symbol> is "=" for public, "@" for secret, "*" for private.
            
        - Did we implement symbol? Possibly not, maybe default "=" for all listed channels, unless we check mode:
            
            - We can derive: if secret flag set and user listing is not a member (shouldn't be listing anyway), but if listing themselves in secret, what symbol? I think if you're in it, symbol still "=" or "@"? Actually RFC says "@" for secret and "*" for private always to differentiate to user that those channels are hidden from others.
                
            - Possibly implement symbol accordingly:
                
                - We might have set prefix in sendNumeric for 353 as param[0] (and used "=" by default).
                    
                - Let's say we used "=" for all or tried to be correct.
                    
                - We can test: if channel was secret, did we mark prefix differently?
                    
                - If not implemented, it's okay (client might not rely heavily, but standard clients do show it differently).
                    
                - For test, assume we did simpler: always "=".
                    
                - Or if implemented: test secret channel listing for member yields "@".
                    
        - The list of names must have appropriate "@" or "+" in front of nicks of ops/voiced.
            
            - Confirm that for an op user we included "@".
                
            - If voice, included "+".
                
            - If none, just name.
                
        - Check that trailing formatting is correct (names preceded by colon and separated by spaces).
            
    - End of list:
        
        - For each channel listed, a 366 should follow.
            
        - If no param and multiple channels, maybe one 366 with "*" to close all lists.
            
        - But easier to implement per channel, except in the case of no param where we iterate all channels:
            
            - We could send a 366 per channel as well, but that might confuse some clients seeing duplicate "end of names" lines, not sure.
                
            - Typically with no param, clients expect one 366 after all, I think with target "*".
                
            - Did we account for that? Possibly yes as we wrote for no param to do one at end with "*".
                
            - We'll adjust check:
                
                - if no param, expect one 366 with "*".
                    
                - If param, expect per channel 366.
                    
10. **PRIVMSG/NOTICE command**:
    

- Setup: Users Alice, Bob, and #chan1 with them.
    
- **Private message user**:
    
    - Alice sends `PRIVMSG Bob :Hello`.
        
        - Bob should receive: `:Alice!user@host PRIVMSG Bob :Hello`.
            
        - Alice's buffer: no reply (client will show it as sent in their UI).
            
        - If Bob is away (set Bob.awayMessage = "BRB"):
            
            - Alice should get `301 Bob :BRB` (RPL_AWAY).
                
            - We planned to implement away optional, but mention in test.
                
            - If not done, skip.
                
        - If Bob not online:
            
            - Alice gets `401 Bob :No such nick`.
                
            - Confirm error format.
                
    - Alice sends `NOTICE Bob :Hey`.
        
        - Bob receives `:Alice!user@host NOTICE Bob :Hey`.
            
        - Alice receives nothing, and if Bob didn't exist:
            
            - Alice should NOT get 401 (since it's notice).
                
            - We test: send to a non-user:  
                ~ ensure no error: check Alice buffer unchanged.
                
            - If Bob is away, no away response because it's notice.
                
            - If any error conditions (like self target? That isn't an error, likely just delivered to self or ignore).
                
                - If we choose to allow self-target:
                    
                    - Alice sends PRIVMSG Alice: possibly the server will just deliver to her (some do).
                        
                    - We can either drop or deliver, not crucial.
                        
                    - If delivered, she'll receive her own message echoed (maybe unusual).
                        
                    - If dropped, no notify (likely do nothing).
                        
                    - We skip unless implemented explicitly.
                        
- **Message to channel**:
    
    - Alice sends `PRIVMSG #chan1 :Hello group`.
        
        - All members except Alice should get it:
            
            - Bob gets `:Alice!user@host PRIVMSG #chan1 :Hello group`.
                
            - If any others, they'd get same.
                
            - Alice does not get her own message from server, so her buffer remains unchanged (the client itself will show what it sent).
                
        - If channel mode +n (no external) and Alice is a member, it's fine.
            
        - If she weren't a member:
            
            - Example: Charlie not in #chan1 tries `PRIVMSG #chan1 :Hi`.
                
            - Channel is likely +n by default (we might have set).
                
            - So Charlie should get `404 #chan1 :Cannot send to channel`.
                
            - Channel members do not receive anything.
                
        - If channel moderated +m:
            
            - If Alice not voice/op:  
                ~ She tries to send, she should get `404 ... :Cannot send to channel`.  
                ~ Bob (op maybe) does not get message because it was blocked.
                
            - If Alice is op or voice, it goes through normally.
                
            - If channel moderated, test both a voice user and a non-voice user sending:  
                ~ Non-voice gets error (if not suppressed).  
                ~ If we suppressed external and moderated errors for notice, test those:
                
                - Charlie (not in channel) NOTICE the channel:
                    
                    - Should be silently dropped (Charlie's buffer unchanged, others nothing).
                        
                - Alice (not voice, moderated) NOTICE channel:
                    
                    - She isn't allowed to speak either, but since it's notice, the server should drop it without error.
                        
                    - Test: no error to Alice, no message delivered.
                        
            - If voice user sends:  
                ~ goes through with no hindrance.  
                ~ test Bob gives voice to Alice, then Alice sends message, Bob receives it now.
                
    - If channel has +b ban on a user who is not in channel sending external:
        
        - If channel not +n, an external message might be allowed normally.
            
        - But if the user matches a +b ban, some servers might still block external. It's not in RFC explicitly. We didn't implement separate check likely.
            
        - We can ignore this rare scenario.
            
    - If multiple recipients:
        
        - Alice `PRIVMSG Bob,#chan1 :Hello all`.
            
            - Bob should get a private message (once).
                
            - Channel members get channel msg.
                
            - If one of the targets fails (like if one nick doesn't exist):  
                ~ Possibly send one 401 for that nick but still deliver to others.  
                ~ E.g. `PRIVMSG Bob,NoOne :Hi`.
                
                - Bob gets message.
                    
                - Alice gets 401 for "NoOne".
                    
            - If multiple channels:  
                ~ deliver to each accordingly.
                
            - If duplicates (like same user appears twice or user also in channel):  
                ~ Should avoid sending twice.  
                ~ If we didn't explicitly code duplication check, a user in two target channels might get duplicate messages.  
                ~ Check if we considered this: likely not implemented. Could test:
                
                - Bob is in #chan1.
                    
                - Alice sends to "Bob,#chan1".
                    
                - Without de-dup, Bob might get one direct and one via channel.
                    
                - Many servers do attempt to avoid duplicate by tracking targets.
                    
                - We might not have.
                    
                - If we didn't, Bob sees two messages (one addressed to him, one to channel from Alice).
                    
                - That's not ideal, but if we didn't handle, mention as minor flaw.  
                    ~ If we did implement a set for delivered targets in handlePrivmsg, then Bob should get only one.  
                    ~ It's advanced, probably we didn't do that.
                    
            - We can simulate and see what we would have done:  
                likely we didn't deduplicate. It's optional but good to mention.
                
- **Maximum line length**:
    
    - Not easily testable here, but we might simulate a very long message near 512 bytes:
        
        - ensure it's truncated or handled by socket layer.
            
        - Not required to test if not implemented explicitly.
            
- **Edge**:
    
    - Not registered user sending PRIVMSG:
        
        - The server should block any command by unregistered: earlier we set to send ERR_NOTREGISTERED 451 for such attempts.
            
        - Simulate: user with isRegistered false tries PRIVMSG:
            
            - Expect 451.
                
            - That covers one more general case: test that 451 is returned for any command (JOIN, PRIVMSG, etc) if user not fully reg.
                
            - We could test with an unregistered dummy user calling one of these handlers, but in practice, our design might have prevented the call if we check in main dispatch.
                
            - But we can simulate by calling handlePrivmsg on an unregistered user pointer:  
                ~ If in handlePrivmsg we don't check, then nothing stops them, which is a bug since we expected main logic to check.  
                ~ If main logic does check (like we said in general design), then these handlers won't be called.  
                ~ So the test would be at integration level: try to send a command before registering.  
                ~ Possibly simulate a raw input from an unregistered connection: "JOIN #chan"
                
                - main sees user.isRegistered false and command not PASS/NICK/USER, so send 451.
                    
                - That is integration rather than unit test for each command.
                    
            - We can incorporate in integration test below.
                

### Pruebas de Escenarios Completos (Integración)

Estas pruebas simulan secuencias de acciones en orden para verificar que la interacción global funciona como esperado y que las condiciones límite se manejan bien en contexto. Incluir errores y recuperaciones.

1. **Escenario registro simple**:
    
    - Simular conexión de un cliente nuevo:
        
        - Send `NICK testuser` then `USER test 0 * :Test User`.
            
        - Ensure server doesn't fully register on NICK alone (no welcome).
            
        - After USER, expect welcome sequence: 001,002,003,004, possibly 005, 375-376 for MOTD.
            
        - Confirm user can then join channel or get error if try command before registration:
            
            - e.g. if they send `JOIN #chan` before USER, server should respond 451 Not registered (which we covered).
                
            - Test: call handleJoin on a not registered user:
                
                - If our design had main guard, handleJoin might not even be called. But to test from high-level:
                    
                - simulate parse of "JOIN #chan", see user not reg, generate 451.
                    
                - Confirm that.
                    
        - If PASS is required (not in our config default maybe, but test optional):
            
            - If we had a server password and user didn't send PASS first:
                
                - After registration, server might disconnect them or send "464 :Password incorrect" if wrong.
                    
                - This optional path if implemented.
                    
                - We'll skip if not implemented, or test if we did: perhaps with a config requiring "letmein".
                    
                - If so, simulate user not sending PASS:  
                    ~ After NICK/USER, server would send 464 and close connection, no welcome.
                    
                - If sending correct PASS then NICK/USER, normal flow continues.
                    
2. **Escenario de conversación en canal**:
    
    - Users: Alice, Bob connect and join #room.
        
    - Steps:
        
        - Alice NICK/USER -> welcome.
            
        - Bob NICK/USER -> welcome.
            
        - Alice JOIN #room (channel created, she is op).
            
            - She gets join confirm + names (just her).
                
        - Bob JOIN #room (Alice sees join, Bob gets names).
            
            - Confirm each sees correct outputs.
                
        - Alice: PRIVMSG #room "Hello everyone".
            
            - Bob receives message.
                
            - Alice sees nothing new from server but presumably has local echo.
                
        - Bob: PRIVMSG #room "Hi Alice".
            
            - Alice receives.
                
        - Bob: set himself away: (if AWAY implemented).
            
            - Alice sends PRIVMSG Bob "Are you there?"
                
            - Alice receives 301 away.
                
        - Alice changes topic: `TOPIC #room :Discussion`.
            
            - Bob sees the TOPIC change.
                
            - Query by Bob yields correct topic.
                
        - Bob tries an unauthorized action: e.g. `MODE #room +k secret` (Bob not op):
            
            - Bob gets ERR_CHANOPRIVSNEEDED.
                
            - Channel unchanged.
                
        - Alice op gives Bob voice: `MODE #room +v Bob`.
            
            - Bob sees mode change.
                
        - Channel moderation on: `MODE #room +m`.
            
            - Both see +m.
                
            - Bob (voice) sends message, should go through.
                
            - Alice (op) sends, goes through.
                
            - If there was a Charlie not voice, they'd be blocked.
                
        - Alice kicks Bob:
            
            - Bob gets KICK and is removed.
                
            - Alice sees KICK message too.
                
            - Bob's client would auto leave channel UI.
                
        - Bob (outside now) tries to send message to channel:
            
            - Since not a member and channel likely +n set by default (if we did), Bob gets ERR_CANNOTSENDTOCHAN.
                
        - Alice invites Bob back:
            
            - Bob gets invite.
                
            - Bob joins again successfully.
                
            - States consistent (he's no longer op or voice unless preserved? (We probably removed those on kick).
                
            - So join as normal member.
                
        - Clean up: Alice and Bob quit.
            
            - Each should generate QUIT messages to the other (when Alice quits, Bob sees it; when Bob quits, no one left to see, just channel removed if empty).
                
    - Check final state: channel removed if empty, users removed.
        
3. **Escenario errores**:
    
    - Already covered many error cases in units. Summarize:
        
        - Using a command unknown: "FOO bar" -> ERR_UNKNOWNCOMMAND.
            
        - Sending commands out of sequence:
            
            - e.g. send PRIVMSG without registering -> ERR_NOTREGISTERED.
                
            - PASS after registration -> ERR_ALREADYREGISTRED.
                
        - Too many channels:
            
            - set a low limit, join sequentially until limit, next join gets error.
                
            - Possibly join attempt with comma list where one puts user over limit mid command:
                
                - e.g. limit 2, user in 1, tries "JOIN #a,#b,#c".
                    
                - After #a (2nd channel) succeed, now at limit.
                    
                - #b (#3rd channel attempt) should error 405 and skip, #c not attempted or also error.
                    
                - See how we handle partial in multi-join. Possibly stops after hitting error or continues:
                    
                - Likely continues trying others, but after hitting limit it will fail all anyway.
                    
        - No such nick:
            
            - send DM to nonexistent nick, error 401.
                
            - whois on nonexistent if implemented (didn't implement WHOIS).
                
            - invite nonexistent (tested).
                
        - Trying to change modes in invalid ways (tested above).
            
        - Nick collisions:
            
            - two users connect with same nick around same time:
                
                - If one hasn't finished registration and second tries same nick, second should get 433.
                    
                - If both finish reg on separate servers, collision resolution would kill one, but in single server scenario just prevent duplicates.
                    
                - We did handle duplicates at nick command.
                    
            - Also case insensitivity:
                
                - One user "Nick", another tries "nick" -> should treat as conflict.
                    
                - Test: Alice "Nick", Bob tries NICK "nick":  
                    = Bob gets 433 nickname in use.
                    
        - Inconsistent modes:
            
            - Trying to set +p and +s together (if implemented check):
                
                - Possibly we disallow by toggling one off.
                    
                - Try "MODE #chan +ps":
                    
                    # Our code likely sets both because we didn't special-case it in logic unless optional.
                    
                    # It's okay to test if no special handling, then channel ends up with both (which is against RFC).
                    
                    # If optional fix implemented:
                    
                    - maybe we do: if adding +s we remove +p internally and also output it.
                        
                    - Test scenario:  
                        ~ Channel is +p, Alice `MODE #chan +s`.  
                        ~ Expect output maybe "+s" and channel now only +s.  
                        ~ Or output "+s -p".
                        
                    - If not implemented, channel ends +p+s, and we would highlight that in semantic validation.
                        
                - Alternatively, try to set both in one go:  
                    ~ Already covered in mode combination test above.
                    
        - Logging:
            
            - If we implemented logging, check that certain actions wrote to log (not via user-facing output, but maybe a log file or console).
                
            - Could simulate by capturing stdout or log file entries if needed.
                
            - E.g. After a user quits, check log contains "User X quit".
                
            - After channel creation, log has "Channel #name created".
                
            - etc. Only if we implement logging with a testable interface.
                

### Pruebas de Validaciones Semánticas (Opcionales)

Focus on things that are not strictly functional but ensure consistency:

- **Máximo de canales**: Already tested above in errors scenario.
    
- **Nicknames únicos (case-insensitive)**:
    
    - Already tested: Bob tries nick "Alice" when "Alice" exists (433).
        
    - Also test with different case:
        
        - "ALICE" vs "Alice". Should also conflict.
            
        - Use two dummy user objects: one registers "Foobar", second tries "foobar".
            
        - Should conflict as well.
            
    - And no user can be "anonymous":
        
        - Try to nick "anonymous":
            
            - If channel anonymity mode in effect (someone toggled +a on some channel), our server might globally disallow new nick "anonymous".
                
            - We can simulate: set a global flag if any channel has anon mode; easier just not allow at any time:
                
            - If implemented, test that user gets ERR_ERRONEUSNICKNAME or some specific error (some networks have 436 ERR_NICKCOLLISION or 437 ERR_UNAVAILRESOURCE "Nick reserved" etc. But simpler is use 432 or 433 to indicate not allowed).
                
            - If not implemented, skip.
                
- **Modos inconsistentes**:
    
    - Already touched: +p and +s should not coexist.
        
    - If our server doesn't enforce, at least note violation.
        
    - If it does:
        
        - Set channel +p, then try +s:
            
            - Ideally result: channel ends up only with +s.
                
            - Or sets both (bad).
                
        - We can either test that after second command, one of the flags is off:
            
            - If we purposely code that setting +s clears +p:  
                ~ test initial +p, then +s yields channel.flags: secret=true, private=false.  
                ~ And mode broadcast possibly included "-p".
                
            - If not coded, test fails on RFC but not on our code if we allowed both. (We then mention improvement).
                
    - Channel +a (anonymous) usage:
        
        - If we did anything: e.g. if channel has +a, check that:
            
            - Users cannot have nick "anonymous" (tested above).
                
            - Also if a message is sent in an anonymous channel, server should mask the prefix to "anonymous!anonymous@anonymous".
                
            - We likely did not implement that complexity in our logic.
                
            - If we did, test:  
                ~ Alice and Bob in #chan with +a:  
                ~ Alice sends message, Bob receives prefix "anonymous!anonymous@anonymous PRIVMSG #chan :msg".  
                ~ Alice sees presumably nothing or maybe same masked? She might still see her own nick? Actually RFC says even to them it might be masked. But might not matter.  
                ~ If not implemented, note as improvement.
                
    - Mode +q (quiet) usage:
        
        - If we considered it, it restricts server sending join/part/nick changes to users: the channel appears as if only one user to each user.
            
        - We definitely didn't implement that (very advanced).
            
        - Would skip or note not implemented.
            
    - **Limits**:
        
        - Already tested: channel user limit works to block join over capacity.
            
        - Possibly check that if user leaves and frees slot, another can join.
            
        - e.g. limit 2, two users in, third blocked. If one leaves, third can join now.
            
        - Test quick scenario:
            
            - Channel +l 2, Alice and Bob in it.
                
            - Charlie tries join, gets 471.
                
            - Bob parts, channel now has 1 user.
                
            - Charlie tries again, succeeds now.
                
        - Ensures that our code indeed updated the member count and allowed join next time.
            
    - **Resources**:
        
        - We might simulate heavy loads if performance or memory considered:
            
            - e.g. join 10 channels quickly, ensure user->channels count correct.
                
            - Or create many bans (like 100) and see if store properly. Not crucial.
                
    - **Server reop mode +r**:
        
        - Hard to test, needed in nets with net splits and safe channels.
            
        - We probably skip, just ensure toggling 'r' doesn't break anything:
            
            - e.g. set +r on channel, check we didn't do anything special, just flag stored.
                
            - Shouldn't affect normal operation in our single-server context.
                
    - **WHOIS/WHOWAS**:
        
        - If we had implemented those, test:
            
            - WHOIS returns correct info including channels user shares, etc.
                
            - WHOWAS pulls from history after user quit.
                
            - Likely not implemented.
                
        - But we did store nicknameHistory for user presumably for whowas plan.
            
            - Could test: after user changes nick or quits, the history exists (like user.alts).
                
            - But since we didn't implement actual command, skip.
                

11. **Pruebas de Logging (si aplicable)**:
    

- If we integrated a logging mechanism:
    
    - Trigger some events and inspect logs:
        
        - On new connection (post-registration) should log "User testuser connected from host ...".
            
        - On join channel, log "testuser joined #channel".
            
        - On mode change, log "testuser set +m on #channel".
            
        - On error perhaps log attempts.
            
        - On quit, log "testuser quit (message)".
            
    - How to capture: if we wrote to console via std::cout or to a file, in test either redirect cout to a buffer or open the log file.
        
        - If to file, read file content after events.
            
        - If to cout, use an IO redirect or just manually verify output in a manual test scenario.
            
    - Given this is a spec, we might not actually implement logging output, but at least mention testing by manual observation or using a mock logger if we did injection.
        
    - For spec, just ensure we say "verify logs show entries with correct info and format".
        
- Also test that logging respects configured level:
    
    - e.g. set logLevel = INFO, debug messages not shown, etc., if we had that.
        
- If any sensitive info like passwords, ensure logging sanitized if needed.
    

All these tests ensure the atomic tasks collectively produce an IRC server that adheres to expected behavior.

The tests above illustrate typical success scenarios and error conditions, verifying both correct state changes and proper user/server interactions.