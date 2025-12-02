#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
//c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp Server_Mock.cpp  ../Authentication/User.cpp  Channels/Channel.cpp -I../Authentication/ -IChannels/

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

int main(void) {
    Server server;
    MessageIn in;
    User      senderU;

    init_server(server, "1234");
    
    std::string packet = "PASS 1234\r\n";

    if (!prepare_message(packet, server, in))
        return 0;

    handle_message(in, server, packet);

    packet = "NICK rata\r\n";

    if (!prepare_message(packet, server, in))
        return 0;

    handle_message(in, server, packet);

    std::cout << "el nick del sender dentro del main de prueba de messaging es --> " << server.get_user_by_id(in.sender_id).get_nick() << std::endl;

    packet = "USER Rata 0 * :ratata\r\n";

    if (!prepare_message(packet, server, in))
        return 0;

    handle_message(in, server, packet);

    packet = "PING rata\r\n";
    
    if (!prepare_message(packet, server, in))
        return 0;

    handle_message(in, server, packet);
}