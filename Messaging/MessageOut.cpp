#include "MessageOut.hpp"

MessageOut::~MessageOut() {}

void	*MessageOut::get_msg() {
	return (static_cast<void*>(msg));
}

void	MessageOut::setTarget(MessageTarget *target) {
	this->target = target;
}

NumericReply	*NumericReplyFactory::create(ReplyCode code, Param *param) {
	switch (code)
	{
		case ERR_ERRONEUSNICKNAME:
			return makeErrErroneusNickname(dynamic_cast<NickParam*>(param));
		case ERR_NONICKNAMEGIVEN:
			return makeErrNoNicknamegiven(dynamic_cast<NickParam*>(param));
		case ERR_NICKNAMEINUSE:
			return makeErrNicknameInUse(dynamic_cast<NickParam*>(param));
		case ERR_UNAVAILRESOURCE:
			return makeErrUnavailResource(dynamic_cast<NickParam*>(param));
		case ERR_RESTRICTED:
			return makeErrRestricted(dynamic_cast<NickParam*>(param));
		case ERR_NEEDMOREPARAMS: //A revisar
			return makeErrNeedMoreParams(param);
		case ERR_ALREADYREGISTRED:
			return makeErrAlredyRegistered(dynamic_cast<UserParam*>(param));
		case ERR_NOORIGIN:
			return makeErrNoOrigin(dynamic_cast<PingPongParam*>(param));
		case ERR_NOSUCHSERVER:
			return makeErrNoSuchServer(dynamic_cast<PingPongParam*>(param));
		default:
			return makeErrUnknownCommand();
	}
}