#include "Message.hpp"

msgTokens   getPARAMS(msgTokens tokens) {
    msgTokens::iterator iter = tokens.begin();
    msgTokens params = tokens;
    params.erase(iter, iter + 1);
    if (params[0].type == CMD)
        params.erase(iter, iter + 1);
    return (params);
}

#include "string.h"
MessageOut  sendNumeric(const std::string user, size_t num) {
    const char    *raton = user.c_str();
    MessageOut out;
    memcpy(out.msg, raton, strlen(raton));
    return (out);
}

MessageOut handleNick(size_t clientId, MessageIn in, Server* server) {
    msgTokens   params = getPARAMS(in.tokens);
    msg_token   nickaname = params[0];

    if (nickaname.type == CRLF)
        return (sendNumeric("Enrique Javier", 431));
    if (nickaname.type == COMMA_LIST
        || !isValidNickName(nickaname.str));
        return (sendNumeric("Enrique Javier", 432));
    
}

fnHandlers::fnHandlers() {
    fun[NICK] = handleNick;
    //Asi con todos ...
}

fnHandlers::~fnHandlers() {
    // Na de na
}