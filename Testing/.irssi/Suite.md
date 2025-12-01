2.1. Registro básico correcto

Ejecuta:

PASS testpass
NICK TestUser
USER test 0 * :Real Test


Verifica rawlog:

001 RPL_WELCOME

002 RPL_YOURHOST

003 RPL_CREATED

004 RPL_MYINFO

MOTD (372/375/376)

Esto es tu baseline.

2.2. Registro sin PASS (si tu server lo requiere)
NICK NoPass
USER nopass 0 * :nopass


Esperado (si PASS obligatorio):

464 :Password required

2.3. Registro con NICK duplicado

En otra ventana de Irssi:

/connect 127.0.0.1 6667
NICK TestUser


Esperado:

433 * TestUser :Nickname is already in use

2.4. Doble USER

Cuando ya estés registrado:

USER fails 0 * :Bad


Esperado:

462 TestUser :Unauthorized command (already registered)

🧪 3. Test Pack – Canales
3.1. JOIN canal
JOIN #test


Espera:

JOIN broadcast a canal

MODE #test +nt (si tu server lo pone por defecto)

NAMES (353)

ENDOFNAMES (366)

3.2. Topic
TOPIC #test :Bienvenidos

3.3. Cambios de nick dentro del canal
NICK NuevoNick


El rawlog debe entregar broadcast del NICK a todo el canal.

🛡 4. Test Pack – Operator workflow

Asumiendo que el creador de canal recibe +o:

4.1. Dar op
MODE #test +o TestUser

4.2. Quitar op
MODE #test -o TestUser

4.3. Voice
MODE #test +v TestUser

4.4. Moderated channel
MODE #test +m

4.5. Intentar hablar sin voice

En otra ventana:

PRIVMSG #test :hola


Esperado:

404 user #test :Cannot send to channel

🔨 5. Test Pack – Bans, exceptions, limits, keys
5.1. Ban con máscara
MODE #test +b *!*@*

5.2. Lista de bans
MODE #test +b


Debe responder:

367 RPL_BANLIST

368 RPL_ENDOFBANLIST

5.3. Canal con clave
MODE #test +k secreto


En otra conexión:

JOIN #test


Esperado:

475 :Cannot join channel (+k)

5.4. Limite de usuarios
MODE #test +l 1


Con dos conexiones:

2ª conexión → intento JOIN → debe fallar con:

471 #test :Cannot join channel (+l)

🎤 6. Test Pack – PRIVMSG / NOTICE / WHO / WHOIS
6.1. Mensajes directos y al canal
PRIVMSG #test :Hola a todos
PRIVMSG OtroUsuario :hola tío

6.2. NOTICE
NOTICE #test :Hola notice


El canal lo recibe sin autorespuesta.

6.3. WHO
WHO #test


Debe devolver la lista con formato:

352 <nick> #test user host server nick H... :<hops> realname
315 <nick> #test :End of WHO list.

6.4. WHOIS
WHOIS TestUser

💀 7. Test Pack – KICK / INVITE
7.1. INVITE
INVITE OtroNick #test


Rawlog debe mostrar:

341 <you> OtroNick #test

7.2. KICK
KICK #test OtroNick :fuera

🔥 8. Test Pack – QUIT y desconexiones
QUIT
QUIT :adiós chavales


Los demás usuarios deben ver:

:Nick!user@host QUIT :adiós chavales