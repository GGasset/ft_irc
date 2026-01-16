#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
// c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp  ../Authentication/User.cpp  Channels/Channel.cpp ../Socket/Server.cpp Nick/Nick.cpp  -I../../Include/Server/ -I../../Include/Channels -I ../../Include/Parsing -I../../Include

/* ---------- 1. INIT SERVER ----------- */
void init_server(Server &server, std::string passw) {
    server.addUser(User("nocambianick", 0));
    server.passw = passw;
}

/* ---------- 2. LEXING + PARSING ----------- */
bool prepare_message(const std::string &packet, Server &server, MessageIn &in) {
    msgTokens tokens;
    ParseStatus status = VALID_MSG;

    tokens = msgTokenizer(packet);
    in = parseMessage(tokens, status);

    if (status != VALID_MSG)
        return false;

    in.sender_id = 0;  // Normalmente te lo pasa GG

    Param *params = ParamsFactory(in.getCommand(), tokens);

    try {
        params->validateParam();
    } catch (Param::BadSyntax &e) {
        MessageOut *ret =
            NumericReplyFactory::create_and_target(e.getErrCode(), server, params,
                std::vector<size_t>{in.sender_id}, 'u');

        std::cout << "[NumericReply]: " << ret->getRpl() << std::endl;
        return false;
    }

    in.setParams(params);
    return true;
}

#include <unistd.h>
/* ---------- 3. HANDLE MESSAGE ----------- */
void handle_message(MessageIn &in, Server &server, const std::string &packet) {

    MessageOut *ret = fnHandlers()(in.getCommand(), in, server);
    if (ret != NULL) {
        std::cout << "Respuesta que se enviaría por el socket ante [packet]: "
                  << packet << " --> " << ret->getRpl() << std::endl;
    }
}

#include <iostream>
#include <vector>

static void send_cmd(Server &server, MessageIn &in, const std::string &cmd)
{
    std::string packet = cmd;

    std::cout << "\n>>> " << packet;

    if (!prepare_message(packet, server, in))
    {
        std::cout << " [prepare_message FAILED]\n";
        return;
    }

    handle_message(in, server, packet);
}

int main(void)
{
    Server    server;
    MessageIn in;

    std::cout << "==== INIT SERVER ====\n";
    init_server(server, "1234");

    /*
    =====================================================
    ESCENARIO 1: Registro correcto de un usuario
    =====================================================
    */

    std::cout << "\n==== SCENARIO 1: REGISTER USER A ====\n";

    send_cmd(server, in, "PASS 1234\r\n");
    send_cmd(server, in, "NICK alice\r\n");
    send_cmd(server, in, "USER alice 0 * :Alice Wonderland\r\n");

    std::cout << "Nick registrado: "
              << server.get_user_by_id(in.sender_id).get_nick()
              << "\n";

    /*
    =====================================================
    ESCENARIO 2: JOIN a canal y mensaje al canal
    =====================================================
    */

    std::cout << "\n==== SCENARIO 2: JOIN CHANNEL ====\n";

    send_cmd(server, in, "JOIN #tea\r\n");
    send_cmd(server, in, "PRIVMSG #tea :Hola a todos desde Alice\r\n");

    /*
    =====================================================
    ESCENARIO 3: Segundo usuario (Bob)
    =====================================================
    */

    std::cout << "\n==== SCENARIO 3: REGISTER USER B ====\n";

    MessageIn in_bob;

    send_cmd(server, in_bob, "PASS 1234\r\n");
    send_cmd(server, in_bob, "NICK bob\r\n");
    send_cmd(server, in_bob, "USER bob 0 * :Bob Builder\r\n");

    std::cout << "Nick registrado (bob): "
              << server.get_user_by_id(in_bob.sender_id).get_nick()
              << "\n";

    /*
    =====================================================
    ESCENARIO 4: Bob entra al canal y habla
    =====================================================
    */

    std::cout << "\n==== SCENARIO 4: BOB JOINS CHANNEL ====\n";

    send_cmd(server, in_bob, "JOIN #tea\r\n");
    send_cmd(server, in_bob, "PRIVMSG #tea :Hola Alice!\r\n");

    /*
    =====================================================
    ESCENARIO 5: Mensajes privados
    =====================================================
    */

    std::cout << "\n==== SCENARIO 5: PRIVATE MESSAGES ====\n";

    send_cmd(server, in,     "PRIVMSG bob :Ey Bob, bienvenido\r\n");
    send_cmd(server, in_bob, "PRIVMSG alice :Gracias Alice 😄\r\n");

    /*
    =====================================================
    ESCENARIO 6: TOPIC y MODE básicos
    =====================================================
    */

    // std::cout << "\n==== SCENARIO 6: CHANNEL MODES ====\n";

    // send_cmd(server, in, "TOPIC #tea :Mesa del té infinita\r\n");
    // send_cmd(server, in, "MODE #tea +i\r\n");

    /*
    =====================================================
    ESCENARIO 7: PING / PONG
    =====================================================
    */

    std::cout << "\n==== SCENARIO 7: PING ====\n";

    send_cmd(server, in, "PING alice\r\n");
    send_cmd(server, in_bob, "PING bob\r\n");

    /*
    =====================================================
    ESCENARIO 8: PART y re-JOIN
    =====================================================
    */

    // std::cout << "\n==== SCENARIO 8: PART / JOIN ====\n";

    // send_cmd(server, in_bob, "PART #tea :Me voy un momento\r\n");
    // send_cmd(server, in_bob, "JOIN #tea\r\n");

    /*
    =====================================================
    ESCENARIO 9: QUIT limpio
    =====================================================
    */

    std::cout << "\n==== SCENARIO 9: QUIT ====\n";

    send_cmd(server, in_bob, "QUIT :Hasta luego\r\n");
    send_cmd(server, in,     "QUIT :Cierro servidor de pruebas\r\n");

    std::cout << "\n==== END OF TESTS ====\n";
    return 0;
}
