#include "Message.hpp"

msgTokens   getPARAMS(msgTokens tokens) {
    msgTokens::iterator iter = tokens.begin();
    msgTokens params = tokens;
    params.erase(iter, iter + 1);
    if (params[0].type == WORD || params[0].type == NUMBER)
        params.erase(iter, iter + 1);
    return (params);
}

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

fnHandlers::fnHandlers() {
    // fun[NICK] = handleNick;
    //Asi con todos ...
}

fnHandlers::~fnHandlers() {
    // Na de na
}