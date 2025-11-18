#include "Message.hpp"


msgTokens   getPARAMS(msgTokens tokens) {
    msgTokens::iterator iter = tokens.begin();
    msgTokens params = tokens;
    params.erase(iter, iter + 1);
    if (params[0].type == WORD || params[0].type == NUMBER)
        params.erase(iter, iter + 1);
    return (params);
}

MessageOut  sendNumeric(const std::string user, size_t num) {
    const char  *raton = user.c_str();
    MessageOut  out;
    memcpy(out.msg, raton, strlen(raton));
    return (out);
}

void    complete_registry(User user) {
    // if (!user.nickname.empty() {
        // || !user.username.empty()
        // || !user.realname.empty()
    // }
        sendNumeric("Enrique Javier", 001);
        sendNumeric("Enrique Javier", 002);
        sendNumeric("Enrique Javier", 003);
        sendNumeric("Enrique Javier", 004);
}

MessageOut handleNick(size_t clientId, MessageIn in, Server &server) {
    msgTokens   params = getPARAMS(in.tokens);
    msg_token   nickname = params[0];
    MessageOut  out;
    std::vector<User> clients = server.getUsers(); // En realidad deberia de ser el vector del servidor.

    if (nickname.type == CRLF)
        return (sendNumeric("Enrique Javier", 431));
    if (nickname.type == COMMA_LIST
        || !isValidNickName(nickname.str))
        return (sendNumeric("Enrique Javier", 432));
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNick() == nickname.str)
            return (sendNumeric("Enrique Javier", 433));
    }
    clients[clientId].setNick(nickname.str);
    std::string servername = "irc.local"; // Deberia ser el del servidor, extraido del archivo de configuracion.
    out.fillMsgOut(clients[clientId], servername, "NICK", nickname.str);
    
    out.id = clients[clientId].get_joined_channel();
    complete_registry(clients[clientId]);
    return (out);// Se supone que hay que mandarlo por el socket.
}

MessageOut  handleUser(size_t clientId, MessageIn in, Server &server) {
    msgTokens   params = getPARAMS(in.tokens);
    std::vector<User> clients = server.getUsers(); // En realidad deberia de ser el vector del servidor.

    if (params.size() != 5) // 5 contando con CRLF, en realidad 4.
        return (sendNumeric("Enrique Javier", 461));
    // if (clients[clientId].id == -1)
    //     return (sendNumeric("Enrique Javier", 462));
    // clients[clientId].setUserName(params[0]);
    // clients[clientId].setRealName(params[0]);
    complete_registry(clients[clientId]);
    return (sendNumeric("Enrique Javier", 555)); // Provisional. EL numero no significa nada.
}

// MessageOut	handleQuit(size_t clientId, MessageIn in, Server &server) {

// }

fnHandlers::fnHandlers() {
    fun[NICK] = handleNick;
    //Asi con todos ...
}

fnHandlers::~fnHandlers() {
    // Na de na
}