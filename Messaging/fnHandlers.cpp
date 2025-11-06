#include "Message.hpp"

msgTokens   getPARAMS(msgTokens tokens) {
    msgTokens::iterator iter = tokens.begin();
    msgTokens params = tokens;
    params.erase(iter, iter + 1);
    if (params[0].type == CMD)
        params.erase(iter, iter + 1);
    return (params);
}

MessageOut  sendNumeric(const std::string user, size_t num) {
    const char    *raton = user.c_str();
    MessageOut out;
    memcpy(out.msg, raton, strlen(raton));
    return (out);
}

MessageOut handleNick(size_t clientId, MessageIn in, Server &server) {
    msgTokens   params = getPARAMS(in.tokens);
    msg_token   nickname = params[0];
    MessageOut  out;
    std::vector<User> clients = server.getUsers(); // En realidad deberia de ser el vector del servidor.

    if (nickname.type == CRLF)
        return (sendNumeric("Enrique Javier", 431));
    if (nickname.type == COMMA_LIST
        || !isValidNickName(nickname.str));
        return (sendNumeric("Enrique Javier", 432));
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNick() == nickname.str)
            return (sendNumeric("Enrique Javier", 433));
    }
    clients[clientId].setNick(nickname.str);
    std::string servername = "irc.local"; // Deberia ser el del servidor, extraido del archivo de configuracion.
    out.fillMsgOut(clients[clientId], servername, "NICK", nickname.str);
    // sendChanels(out);
}

fnHandlers::fnHandlers() {
    fun[NICK] = handleNick;
    //Asi con todos ...
}

fnHandlers::~fnHandlers() {
    // Na de na
}