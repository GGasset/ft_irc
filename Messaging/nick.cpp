#include "Message.hpp"

MessageOut handleNick(size_t clientId, MessageIn in, Server &server) {
    msgTokens   params = getPARAMS(in.tokens);
    msg_token   nickToken = params[0];
    MessageOut  out;
    std::vector<User> clients = server.getUsers(); // En realidad deberia de ser el vector del servidor.

    if (nickToken.type == CRLF)
        return (sendNumeric("Enrique Javier", 431));
    if (nickToken.type == COMMA_LIST
        || !isValidNickName(nickToken.str))
        return (sendNumeric("Enrique Javier", 432));
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNick() == nickToken.str)
            return (sendNumeric("Enrique Javier", 433));
    }
    clients[clientId].setNick(nickToken.str);
    std::string servername = "irc.local"; // Deberia ser el del servidor, extraido del archivo de configuracion.
    out.ids = clients[clientId].get_joined_channels();
    out.fillMsgOut(clients[clientId], servername, "NICK", nickToken.str);
    
    complete_registry(clients[clientId]);
    return (out);// Se supone que hay que mandarlo por el socket.
}