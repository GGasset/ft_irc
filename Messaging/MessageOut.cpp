#include "MessageOut.hpp"

MessageOut::~MessageOut() {}

void	*MessageOut::get_msg() {
	return (static_cast<void*>(msg));
}

void	MessageOut::setTarget(MessageTarget *target) {
	this->target = target;
}

NumericReply	*NumericReplyFactory::create(ReplyCode code, Server &serv, Param *param) {
	switch (code)
	{
		case RPL_WELCOME:
			return makeRplWelcome(serv, dynamic_cast<UserParam*>(param));
		case RPL_YOURHOST:
			return makeRplYourHost(serv, dynamic_cast<UserParam*>(param));
		case RPL_CREATED:
			return makeRplCreated(serv, dynamic_cast<UserParam*>(param));
		case ERR_ERRONEUSNICKNAME:
			return makeErrErroneusNickname(serv, dynamic_cast<NickParam*>(param));
		case ERR_NONICKNAMEGIVEN:
			return makeErrNoNicknamegiven(serv, dynamic_cast<NickParam*>(param));
		case ERR_NICKNAMEINUSE:
			return makeErrNicknameInUse(serv, dynamic_cast<NickParam*>(param));
		case ERR_UNAVAILRESOURCE:
			return makeErrUnavailResource(serv, dynamic_cast<NickParam*>(param));
		case ERR_RESTRICTED:
			return makeErrRestricted(serv, dynamic_cast<NickParam*>(param));
		case ERR_NEEDMOREPARAMS: //A revisar
			return makeErrNeedMoreParams(serv, param);
		case ERR_ALREADYREGISTRED:
			return makeErrAlredyRegistered(serv, dynamic_cast<UserParam*>(param));
		case ERR_NOORIGIN:
			return makeErrNoOrigin(serv, dynamic_cast<PingPongParam*>(param));
		case ERR_NOSUCHSERVER:
			return makeErrNoSuchServer(serv, dynamic_cast<PingPongParam*>(param));
		default:
			return makeErrUnknownCommand(serv);
	}
}

NumericReply *NumericReplyFactory::create_and_target(ReplyCode code, Server &serv, Param *param, std::vector<size_t> ids, char t) {
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
		default:
			return NULL;
	}
}
