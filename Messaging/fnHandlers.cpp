#include "fnHandlers.hpp"

void    complete_registry(User user, Server &server, UserParam *param) {
    if (!user.get_nick().empty()
    && !user.getUsername().empty()
    && !user.getRealname().empty()) {
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

    for (size_t i = 0; i < clients.size(); i++) {
        std::string nickClient;
        nickClient = clients[i].get_nick();
        if (nickClient == np->nickname) {
            MessageOut *ret = NumericReplyFactory::create(ERR_NICKNAMEINUSE, server, np);
            ret->setTarget(
                MessageTargetFactory::create(server, 
                                            (std::vector<size_t>){in.sender_id},
                                            'u')
            );
            return ret;
        }
    }
    senderU.setNick(np->nickname);
    MessageOut *nickBroadcast = ForwardedCommandFactory::create(NICK, server, np);
    nickBroadcast->setTarget(
        MessageTargetFactory::create(server, 
                            senderU.get_joined_channels(),
                            'c')
    );
    return (nickBroadcast);
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