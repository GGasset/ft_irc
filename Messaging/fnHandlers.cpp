#include "fnHandlers.hpp"

void    complete_registry(User user, Server &server, UserParam *param) {
    if (user.is_registered()) {

        MessageTarget   *target; MessageTargetFactory::create(server, 
                                    (std::vector<size_t>){(unsigned long) user.get_id()},
                                    'u'
        );
        NumericReply    *reg;
        reg = NumericReplyFactory::create(RPL_WELCOME, server, param);
        reg->setTarget(target), reg->deliver();

        reg = NumericReplyFactory::create(RPL_YOURHOST, server, param);
        reg->setTarget(target), reg->deliver();

        reg = NumericReplyFactory::create(RPL_CREATED, server, param);
        reg->setTarget(target), reg->deliver();
        
        // reg = NumericReplyFactory::create(RPL_MYINFO, server, param);
        // reg->setTarget(target), reg->deliver();
    }
}

MessageOut *handleNick(MessageIn in, Server &server) {
    NickParam           *np = dynamic_cast<NickParam*>(in.getParams());
    std::vector<User>   clients = server.getUsers(); // En realidad deberia de ser el vector del servidor.
    User                senderU;
    
    senderU = clients[in.sender_id];

    MessageTarget   *client_target = MessageTargetFactory::create(server, (std::vector<size_t>){in.sender_id}, 'u');

    for (size_t i = 0; i < clients.size(); i++) {
        std::string nickClient;
        nickClient = clients[i].get_nick();
        if (nickClient == np->nickname) {
            MessageOut *ret = NumericReplyFactory::create(ERR_NICKNAMEINUSE, server, np);
            ret->setTarget(client_target);
            return ret;
        }
    }
    
    /* Necesidad de crear el ERR_UNAVAILRESOURCE, que gestiona si el nombre colisiona en la historia de nick's. */
    std::vector<std::string> nick_h = server.get_nick_history();
    for (size_t i = 0; i < nick_h.size(); i++) {
        if (clients[i].get_nick() == nick_h[0]) {
            MessageOut  *ret = NumericReplyFactory::create(ERR_UNAVAILRESOURCE, server, np);
            ret->setTarget(client_target);
            return ret;
        }
    }

    senderU.setNick(np->nickname);
    if (!server.get_user_by_id(in.sender_id).is_registered())
        return NULL;
    MessageOut *nickBroadcast = ForwardedCommandFactory::create(NICK, server, np);
    nickBroadcast->setTarget(
        MessageTargetFactory::create(server, senderU.get_joined_channels(), 'c')
    );
    return (nickBroadcast);
}

// MessageOut  *handleUser(MessageIn in, Server &server) {

// }

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