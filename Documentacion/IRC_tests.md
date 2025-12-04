🧱 TESTPACK EXTENDIDO – IRC FULL SUITE (Libera.Chat)

Este TestPack está dividido por capas:

🔵 Registro

🟣 Canales (creación, modos, op flow)

🟠 Privilegios y errores

🟤 WHO / WHOIS / NAMES / LIST

🟢 Bans + exceptions + invites

🔴 Operator Wars (peleas de modos)

🟡 Mensajería avanzada

🔥 Edge cases (multiconexión, flood, errcodes, etc.)

Todos los comandos están pensados para ejecutarlos dentro de Irssi con rawlog activado.

🔵 1. Registro – flujo completo
1.1. Registro normal
PASS foo
NICK Test001
USER test 0 * :Test User 001


Verificar:

001 WELCOME

002 YOURHOST

003 CREATED

004 MYINFO

375–376 MOTD

1.2. Cambio de nick temprano
NICK TestAAAA
NICK TestBBBB


Debe verse el broadcast correspondiente.

1.3. Registro sin PASS (si Libera lo permite)
NICK NoPass
USER blah 0 * :sin pass


Libera normalmente no exige PASS → debería registrar.

1.4. Registro con nick duplicado

Segunda ventana:

NICK Test001


Esperado:

433 * Test001 :Nickname is already in use

1.5. Doble USER → error
USER fail 0 * :fail


Esperado:

462 :Unauthorized command (already registered)

🟣 2. Canales – flujo completo
2.1. Crear canal
JOIN #test42


Esperado:

JOIN broadcast

MODE #test42 +nt

353 NAMES

366 ENDOFNAMES

2.2. Safe channel creation
JOIN !foo


Debería crearse:

!ABCDEfoo


Con MODE:

MODE !ABCDEfoo


Ver:

+O Test001 +o Test001

2.3. Topic
TOPIC #test42 :Canal oficial de testing

2.4. Múltiples usuarios uniendo y saliendo

Ventana 2:

JOIN #test42


Ventana 1:

PART #test42 :bye
JOIN #test42


Verifica:

JOIN/PART

RPL_JOIN

prefijos correctos

🟠 3. Modos de canal (FULL)
3.1. Dar op / quitar op
MODE #test42 +o Test001
MODE #test42 -o Test001

3.2. Voice
MODE #test42 +v Test002

3.3. Canal moderado
MODE #test42 +m


Ventana 2:

PRIVMSG #test42 :hola


Esperado:

404 :Cannot send to channel

3.4 Invisible channel
MODE #test42 +i


Ventana 2:

LIST


#test42 debería NO aparecer.

3.5. Canal con clave
MODE #test42 +k pass123


Ventana 2:

JOIN #test42


Esperado:

475 #test42 :Cannot join channel (+k)

3.6. Limite de usuarios
MODE #test42 +l 1


Ventana 2:

JOIN #test42


Esperado:

471 #test42 :Cannot join channel (+l)

🟤 4. WHO / WHOIS / LIST / NAMES
WHO del canal
WHO #test42

WHOIS
WHOIS Test001

NAMES
NAMES #test42

LIST
LIST

🟢 5. BAN / EXCEPT / INVITE
5.1. Ban simple
MODE #test42 +b *!*@*

5.2. Lista de bans
MODE #test42 +b


Debe dar RPL_BANLIST (367).

5.3. Invite
INVITE Test002 #test42

🔴 6. Operator Wars (modo real)

Esto es EXACTAMENTE lo que pasa en redes reales.

Ventana 1:

MODE #test42 +o Test001


Ventana 2:

MODE #test42 +o Test002


Ventana 1:

MODE #test42 -o Test002


Ventana 2:

MODE #test42 -o Test001


Ver:

Collisions de modos

Propagación correcta

Prefijos correctos

🟡 7. Mensajería avanzada
PRIVMSG
PRIVMSG #test42 :hola chavales

NOTICE
NOTICE #test42 :soy una notice

CTCP (ping)
PRIVMSG Test002 :\001PING 12345\001


Debe responder:

NOTICE Test001 :\001PING 12345\001

🔥 8. Edge cases
8.1. Cambiar nick dentro de canal
NICK NuevoNick

8.2. QUIT
QUIT :me voy chavales


Debe verse el broadcast QUIT en el canal.

🧨 Y ahora sí: SCRIPT PARA AUTOMATIZAR ESTOS TESTS
Este script Irssi está pensado para ejecutarse en Libera.Chat.

Crea el archivo:

~/irssi-testing/.irssi/scripts/testsuite.pl


Contenido:

use strict;
use Irssi;

sub run_tests {
    my ($server) = @_;

    return unless $server;

    Irssi::print("== Iniciando TestSuite IRC ==");
    
    # Registro
    $server->command("PASS testpass");
    $server->command("NICK TestSuite001");
    $server->command("USER testsuite 0 * :Test Suite User");

    Irssi::timeout_add(1500, sub {
        # Crear canal
        $server->command("JOIN #test42suite");
    }, undef);

    Irssi::timeout_add(2500, sub {
        $server->command("MODE #test42suite +nt");
        $server->command("TOPIC #test42suite :Testing topic");
        $server->command("MODE #test42suite +m");
    }, undef);

    Irssi::timeout_add(3500, sub {
        $server->command("MODE #test42suite +l 2");
        $server->command("MODE #test42suite +k clave123");
    }, undef);

    Irssi::timeout_add(4500, sub {
        $server->command("MODE #test42suite +b *!*@*");
        $server->command("MODE #test42suite +b");
    }, undef);

    Irssi::timeout_add(5500, sub {
        $server->command("WHO #test42suite");
        $server->command("NAMES #test42suite");
    }, undef);

    Irssi::timeout_add(6500, sub {
        $server->command("PRIVMSG #test42suite :Mensaje de prueba");
        $server->command("NOTICE #test42suite :Notice de prueba");
    }, undef);

    Irssi::timeout_add(8000, sub {
        Irssi::print("== Test suite finalizada ==");
    }, undef);
}

Irssi::command_bind("runsuite", sub {
    my $server = Irssi::active_server();
    run_tests($server);
});

📌 Cómo usar el script

Entras a Irssi con entorno limpio:

HOME=~/irssi-testing irssi


Cargas el script:

/script load testsuite.pl


Conectas a Libera.Chat:

/connect irc.libera.chat 6667


En cuanto veas NOTICE AUTH, abres rawlog:

/rawlog open libera.raw


Lanzas la suite:

/runsuite


El script ejecuta la batería de tests de forma automatizada con timers.