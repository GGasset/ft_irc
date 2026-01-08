#include "MessageOut.hpp"
#include "Factories.hpp"

MessageOut::~MessageOut() {}

void	*MessageOut::get_msg() {
	return (static_cast<void*>(msg));
}

void	MessageOut::setTarget(MessageTarget *target) {
	this->target = target;
}

NumericReply *NumericReplyFactory::create(ReplyCode code, Server &serv, Param *param) {

    switch (code) {

    /* WELCOME */
    case RPL_WELCOME:      return makeRplWelcome(serv, static_cast<UserParam*>(param));
    case RPL_YOURHOST:     return makeRplYourHost(serv, static_cast<UserParam*>(param));
    case RPL_CREATED:      return makeRplCreated(serv, static_cast<UserParam*>(param));

    /* NICK */
    case ERR_ERRONEUSNICKNAME: return makeErrErroneusNickname(serv, static_cast<NickParam*>(param));
    case ERR_NONICKNAMEGIVEN:  return makeErrNoNicknamegiven(serv, static_cast<NickParam*>(param));
    case ERR_NICKNAMEINUSE:    return makeErrNicknameInUse(serv, static_cast<NickParam*>(param));
    case ERR_UNAVAILRESOURCE:  return makeErrUnavailResource(serv, static_cast<NickParam*>(param));
    case ERR_RESTRICTED:       return makeErrRestricted(serv, static_cast<NickParam*>(param));

    /* GENERIC */
    case ERR_NEEDMOREPARAMS:   return makeErrNeedMoreParams(serv, param);
    case ERR_GENERIC:          return makeErrGeneric(serv);

    /* PASS / USER */
    case ERR_ALREADYREGISTRED: return makeErrAlredyRegistered(serv, param);

    /* PING */
    case ERR_NOORIGIN:      return makeErrNoOrigin(serv, static_cast<PingPongParam*>(param));
    case ERR_NOSUCHSERVER:  return makeErrNoSuchServer(serv, static_cast<PingPongParam*>(param));

    /* PART */
    case ERR_NOTONCHANNEL:  return makeErrNotOnChannel(serv, static_cast<PartParam*>(param));

    /* PRIVMSG */
    case ERR_NORECIPIENT:     return makeErrNoRecipient(serv, static_cast<PrivMsgParam*>(param));
    case ERR_NOTEXTTOSEND:    return makeErrNoTextToSend(serv, static_cast<PrivMsgParam*>(param));
    case ERR_NOSUCHNICK:      return makeErrNoSuchNick(serv, static_cast<PrivMsgParam*>(param));
    case ERR_CANNOTSENDTOCHAN:return makeErrCannotSendToChan(serv, static_cast<PrivMsgParam*>(param));
    case ERR_TOOMANYTARGETS:  return makeErrTooManyTargets(serv, static_cast<PrivMsgParam*>(param));
    case ERR_NOTOPLEVEL:      return makeErrNoTopLevel(serv, static_cast<PrivMsgParam*>(param));
    case ERR_WILDTOPLEVEL:    return makeErrWildTopLevel(serv, static_cast<PrivMsgParam*>(param));

    /* MODE */
    case RPL_CHANNELMODEIS:      return makeRplChannelModeIs(serv, static_cast<ModeParam*>(param));
    case ERR_UNKNOWNMODE:        return makeErrUnknownMode(serv, static_cast<ModeParam*>(param));
    case ERR_UMODEUNKNOWNFLAG:   return makeErrUModeUnknownFlag(serv, static_cast<ModeParam*>(param));
    case ERR_KEYSET:             return makeErrKeySet(serv, static_cast<ModeParam*>(param));
    case ERR_USERNOTINCHANNEL:   return makeErrUserNotInChannel(serv, static_cast<ModeParam*>(param));

    /* INVITE */
    case RPL_INVITING:         return makeRplInviting(serv, static_cast<InviteParam*>(param));
    case ERR_USERONCHANNEL:    return makeErrUserOnChannel(serv, static_cast<InviteParam*>(param));

    /* JOIN */
    case RPL_NOTOPIC:          return makeRplNoTopic(serv, static_cast<JoinParam*>(param));
    case RPL_TOPIC:            return makeRplTopic(serv, static_cast<JoinParam*>(param));
    case ERR_NOSUCHCHANNEL:    return makeErrNoSuchChannel(serv, static_cast<JoinParam*>(param));
    case ERR_TOOMANYCHANNELS:  return makeErrTooManyChannels(serv, static_cast<JoinParam*>(param));
    case ERR_CHANNELISFULL:    return makeErrChannelIsFull(serv, static_cast<JoinParam*>(param));
    case ERR_INVITEONLYCHAN:   return makeErrInviteOnlyChan(serv, static_cast<JoinParam*>(param));
    case ERR_BANNEDFROMCHAN:   return makeErrBannedFromChan(serv, static_cast<JoinParam*>(param));
    case ERR_BADCHANNELKEY:    return makeErrBadChannelKey(serv, static_cast<JoinParam*>(param));
    case ERR_BADCHANMASK:      return makeErrBadChanMask(serv, static_cast<JoinParam*>(param));
    case ERR_UNSUPPORTEDCHANMODE:return makeErrUnsupportedChanMode(serv, static_cast<JoinParam*>(param));

    /* Unknown */
    case ERR_UNKNOWNCOMMAND:  return makeErrUnknownCommand(serv);

    default:
        return NULL;
    }
}


NumericReply *NumericReplyFactory::create_and_target(ReplyCode code, Server &serv, Param *param, std::vector<size_t> ids, char t) {
	NumericReply	*ret = create(code, serv, param);
	MessageTarget	*target = MessageTargetFactory::create(serv, ids, t);
	ret->setTarget(target);
	return (ret);
}

NumericReply *NumericReplyFactory::create_and_target(ReplyCode code, Server &serv, Param *param, size_t id, char t) {
	std::vector<size_t> ids = {id};
	NumericReply	*ret = create(code, serv, param);
	MessageTarget	*target = MessageTargetFactory::create(serv, ids, t);
	ret->setTarget(target);
	return (ret);
}


ForwardedCommand *ForwardedCommandFactory::create(COMMAND cmd, Server &serv, Param *param) {
	switch (cmd)
	{
		case NICK:
			return makeNickForward(serv, dynamic_cast<NickParam*>(param));
		case PONG:
			return makePingPongForward(serv, dynamic_cast<PingPongParam*>(param));
		case QUIT:
            return new QuitForwardedCommand(serv, dynamic_cast<QuitParam*>(param));
        case JOIN:
            return new JoinForwardedCommand(serv, dynamic_cast<JoinParam*>(param));
		case PART:
            return new PartForwardedCommand(serv, static_cast<PartParam*>(param));
        case PRIVMSG:
            return new PrivMsgForwardedCommand(serv, static_cast<PrivMsgParam*>(param));
        case NOTICE:
            return new NoticeForwardedCommand(serv, static_cast<NoticeParam*>(param));
        case TOPIC:
            return new TopicForwardedCommand(serv, static_cast<TopicParam*>(param));
        case INVITE:
            return new InviteForwardedCommand(serv, static_cast<InviteParam*>(param));
        case KICK:
            return new KickForwardedCommand(serv, static_cast<KickParam*>(param));
        case MODE:
            return new ModeForwardedCommand(serv, static_cast<ModeParam*>(param));
		default:
			return NULL;
	}
}
    
ForwardedCommand *ForwardedCommandFactory::create_and_target(COMMAND cmd, Server &serv, Param *param, size_t id, char t) {
	std::vector<size_t> ids = {id};
    ForwardedCommand    *ret = create(cmd, serv, param);
    MessageTarget       *target = MessageTargetFactory::create(serv, ids, t);

    ret->setTarget(target);
    return (ret);
}

// static ForwardedCommand *create_and_target(Server &serv, Param *param, std::vector<size_t> ids, char t) {

// }