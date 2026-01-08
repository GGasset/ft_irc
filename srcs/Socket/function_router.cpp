// #include "Server.hpp"
#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
//c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp Server_Mock.cpp  ../Authentication/User.cpp  Channels/Channel.cpp -I../Authentication/ -IChannels/

/* ---------- 2. LEXING + PARSING ----------- */
bool prepare_message(const std::string &packet, Server &server, MessageIn &in, size_t sender_id) {
    msgTokens tokens;
    ParseStatus status = VALID_MSG;

    tokens = msgTokenizer(packet);
    if (tokens.size() == 0)
        return (false);

    in = parseMessage(tokens, status);

    if (status != VALID_MSG)
        return false;

    in.sender_id = sender_id;

    Param *params = ParamsFactory(in.getCommand(), tokens);

    try {
        params->validateParam();
    } catch (Param::BadSyntax &e) {
        MessageOut *ret =
            NumericReplyFactory::create_and_target(e.getErrCode(), server, params,
                std::vector<size_t>{in.sender_id}, 'u');

        ret->deliver();
        return false;
    }

    in.setParams(params);
    return true;
}

#include <unistd.h>
/* ---------- 3. HANDLE MESSAGE ----------- */
void handle_message(MessageIn &in, Server &server, const std::string &packet) {
    if (in.getCommand() == COMMAND0)
        return ;
    MessageOut *ret = fnHandlers()(in.getCommand(), in, server);
    if (ret != NULL)
        ret->deliver();
}

void Server::route_message(std::string msg, User &sender, size_t user_index) {
	MessageIn in;

	if (prepare_message(msg, *this, in, user_index))
	    handle_message(in, *this, msg);
}