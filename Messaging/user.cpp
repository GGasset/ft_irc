#include "Message.hpp"

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