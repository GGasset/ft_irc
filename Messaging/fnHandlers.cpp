#include "fnHandlers.hpp"

void    complete_registry(User user, Server &server, UserParam *param) {
    bool    passwd_match = user.passwd_match_pop(false);
    user.passwd_match_pop(passwd_match);

    if (user.is_registered() && passwd_match) {

        MessageTarget   *target; MessageTargetFactory::create(server, 
                                    (std::vector<size_t>){(unsigned long) user.get_id()},
                                    'u'
        );
        NumericReply    *reg;


        reg = NumericReplyFactory::create(RPL_WELCOME, server, param);
        reg->setTarget(target);//, reg->deliver();

        std::cout << "reg: " << reg->getRpl();

        reg = NumericReplyFactory::create(RPL_YOURHOST, server, param);
        reg->setTarget(target);//, reg->deliver();
        std::cout << "reg: " << reg->getRpl();

        reg = NumericReplyFactory::create(RPL_CREATED, server, param);
        reg->setTarget(target);//, reg->deliver();
        std::cout << "reg: " << reg->getRpl();

        // reg = NumericReplyFactory::create(RPL_MYINFO, server, param);
        // reg->setTarget(target), reg->deliver();
    }
}

/* Falta gestionar el caso cuando va primero USER  y después NICK. */
MessageOut *handleNick(MessageIn in, Server &server) {
    NickParam           *np = dynamic_cast<NickParam*>(in.getParams());
    User                senderU;
    
    senderU = server.get_user_by_id(in.sender_id);

    for (size_t i = 0; i < server.n_users(); i++) {
        std::string nickClient;
        nickClient = server.get_user_by_id(i).get_nick();
        if (nickClient == np->nickname) {
            MessageOut *ret = NumericReplyFactory::create_and_target(ERR_NICKNAMEINUSE, server, np,
                                                                     (std::vector<size_t>){in.sender_id}, 'u');
            return ret;
        }
    }
    
    /* Necesidad de crear el ERR_UNAVAILRESOURCE, que gestiona si el nombre colisiona en la historia de nick's. */
    std::vector<std::string> nick_h = server.get_nick_history();
    for (size_t i = 0; i < nick_h.size(); i++) {
        if (server.get_user_by_id(i).get_nick() == nick_h[0]) {
            MessageOut  *ret = NumericReplyFactory::create_and_target(ERR_UNAVAILRESOURCE, server, np, in.sender_id, 'u');
            return ret;
        }
    }

    senderU.setNick(np->nickname);
    server.addNickHistory(np->nickname);
    if (!senderU.is_registered() && senderU.are_names_registered()) {
        senderU.register_user();
        complete_registry(senderU, server, UserParam()(senderU));
    }
    else if (!senderU.is_registered())
        return NULL;
    MessageOut *nickBroadcast = ForwardedCommandFactory::create(NICK, server, np);
    nickBroadcast->setTarget(
        MessageTargetFactory::create(server, senderU.get_joined_channels(), 'c')
    );
    return (nickBroadcast);
}

MessageOut  *handleUser(MessageIn in, Server &server) {
    UserParam   *p = dynamic_cast<UserParam*>(in.getParams());
    User        &senderU = server.get_user_by_id(in.sender_id);

    if (senderU.is_registered()) {
        MessageOut  *ret = NumericReplyFactory::create_and_target(ERR_ALREADYREGISTRED, server, p, in.sender_id, 'c');
        return (ret);
    }
    senderU.set_username(p->username);
    senderU.set_realname(p->realname);

    if (!senderU.is_registered() && senderU.are_names_registered()) {
        senderU.register_user();
        complete_registry(senderU, server, p);
    }
    return (NULL);
}

MessageOut  *handlePass(MessageIn in, Server &server) {
    User        &senderU = server.get_user_by_id(in.sender_id);
    PassParam   *p = dynamic_cast<PassParam*>(in.getParams());
    if (p->password != server.passw) {
        return (NumericReplyFactory::create_and_target(ERR_GENERIC, server, p, in.sender_id, 'u'));
    }
    else
        senderU.passwd_match_pop(true);
    return (NULL);
}

MessageOut  *handlePINGPONG(MessageIn in, Server &server) {
    PingPongParam   *p = dynamic_cast<PingPongParam*>(in.getParams());
    
    // Asumimos que un PONG no está mal formado para clientes correctos.
    if (p->command() == PONG)
        return NULL;
    if (p->server2.empty())
        return (ForwardedCommandFactory::create(PONG, server, p));
    else {
        for (size_t i = 0; i < server.n_users(); i++) {
            if (server.get_user_by_id(i).getHostname() == p->server2)
                return (ForwardedCommandFactory::create(PONG, server, p));
        }
    }
    return (NumericReplyFactory::create_and_target(ERR_NOSUCHSERVER, server, p, in.sender_id, 'u'));
}

fnHandlers::fnHandlers() {
    fun[NICK] = handleNick;
    fun[USER] = handleUser;
    fun[PASS] = handlePass;
    fun[PING] = handlePINGPONG;
    fun[PONG] = handlePINGPONG;
    //Asi con todos ...
}

MessageOut  *fnHandlers::operator()(COMMAND cmd, MessageIn in, Server& server) {
    return (fun[cmd](in, server));
}

fnHandlers::~fnHandlers() {
    // Na de na
}