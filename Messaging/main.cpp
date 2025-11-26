#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
//c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp ../Authentication/User.cpp ../Socket/Server.cpp ../Socket/function_router.cpp Channels/Channel.cpp -I../Socket/ -I../Authentication/ -IChannels/

/* ---------- 1. INIT SERVER ----------- */
void init_server(Server &server) {
    server.addUser(User("ratata", 0));
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

    if (in.getCommand() == NICK || in.getCommand() == USER)
        complete_registry();
    if (ret != NULL) {
        std::cout << "Respuesta que se enviaría por el socket ante [packet]: "
                  << packet << " --> " << ret->getRpl() << std::endl;
    }
}

int main(void) {
    Server server;
    MessageIn in_nick;
    MessageIn in_user;
    User      senderU;

    init_server(server);

    std::string packet = "NICK rata\r\n";

    if (!prepare_message(packet, server, in_nick))
        return 0;

    handle_message(in_nick, server, packet);

    packet = "USER Rata 0 * :ratata\r\n";

    if (!prepare_message(packet, server, in_nick))
        return 0;

    handle_message(in_nick, server, packet);
}
