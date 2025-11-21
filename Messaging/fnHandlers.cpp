#include "fnHandlers.hpp"

void    complete_registry(User user) {
    if (!user.getNick().empty()
    && !user.getUsername().empty()
    && !user.getRealname().empty()) {
        // sendNumeric("Enrique Javier", 001);
        // sendNumeric("Enrique Javier", 002);
        // sendNumeric("Enrique Javier", 003);
        // sendNumeric("Enrique Javier", 004);
    }
}

MessageOut *handleNick(MessageIn in, Server &server) {

}


fnHandlers::fnHandlers() {
    fun[NICK] = handleNick;
    //Asi con todos ...
}

MessageOut  *fnHandlers::operator()(COMMAND cmd, MessageIn in, Server& server) {
    return (fun[cmd](in, server));
}

fnHandlers::~fnHandlers() {
    // Na de na
}