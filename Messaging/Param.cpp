#include "Param.hpp"

Param::~Param() {}

NickParam::NickParam(msgTokens tokens): Param(NICK, tokens) {}

void	NickParam::validateParam() {
	int i;
	for (i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == PREFIX
			|| tokens[i].type == WORD
			|| tokens[i].type == NUMBER
			|| tokens[i].type == SPACE)
			continue ;	
	}
	if (tokens[i].type == CRLF)
		throw BadSyntax(NICK, ERR_NONICKNAMEGIVEN);
	nickname = tokens[i].str;
	if (!isValidNickName(tokens[i].str))
		throw BadSyntax(NICK, ERR_ERRONEUSNICKNAME);
}

// NumericReply *NickParam::mapSyntaxErrorToNumeric(int errCode, NumericReplyFactory replyFactory) {
// 	 switch (errCode) {
// 		case ERR_ERRONEUSNICKNAME:
// 			return replyFactory.makeErrNoNicknamegiven(this);

// 		case ERR_NONICKNAMEGIVEN:
// 			return replyFactory.makeErrErroneusNickname(this);
// 		case ERR_GENERIC:
// 			return replyFactory.makeErrUnknownCommand();
// 		default:
// 			return (NULL);
// 	}
// }


UserParam::UserParam(msgTokens tokens): Param(USER, tokens) {}

void	UserParam::validateParam() {
	// Lo que he extraído del RFC es que se traga lo que sea.
	// Solo se gestiona el caso cuando username es incorrecto.
	int i;

	for (i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == PREFIX
			|| tokens[i].type == WORD
			|| tokens[i].type == NUMBER
			|| tokens[i].type == SPACE)
			continue ;	
	}
	tokens[i].type != CRLF ? username = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? usermode = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? unused = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? unused = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	//usermode y unused me la sudan.
	for (i = 0; i < username.size(); i++) {
		if (!isUserChar(username[i]))
			throw BadSyntax(USER, ERR_UNKNOWNCOMMAND);
	}
}

// NumericReply *UserParam::mapSyntaxErrorToNumeric(int errCode, NumericReplyFactory replyFactory) {
// 	 switch (errCode) {
// 		case ERR_NEEDMOREPARAMS:
// 			return replyFactory.makeErrNeedMoreParams(this);
// 		case ERR_GENERIC:
// 			return replyFactory.makeErrUnknownCommand();
// 		default:
// 			return (NULL);
// 	}
// }

PassParam::PassParam(msgTokens tokens): Param(PASS, tokens) {}

void	PassParam::validateParam() {
	int i = 0;

	for (i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == PREFIX
			|| tokens[i].type == WORD
			|| tokens[i].type == NUMBER
			|| tokens[i].type == SPACE)
			continue ;	
	}
	password = tokens[i].str;
	if (password.empty())
		throw BadSyntax(PASS, ERR_UNKNOWNCOMMAND);
}

// NumericReply *PassParam::mapSyntaxErrorToNumeric(int errCode, NumericReplyFactory replyFactory) {
// 	 switch (errCode) {
// 		case ERR_NEEDMOREPARAMS:
// 			return replyFactory.makeErrNeedMoreParams(this);
// 		case ERR_GENERIC:
// 			return replyFactory.makeErrUnknownCommand();
// 		default:
// 			return (NULL);
// 	}
// }

PingPongParam::PingPongParam(msgTokens tokens): Param(PING, tokens) {}

void	PingPongParam::validateParam() {
	int i = 0;

	for (i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == PREFIX
			|| tokens[i].type == WORD
			|| tokens[i].type == NUMBER
			|| tokens[i].type == SPACE)
			continue ;	
	}
	if (tokens[i].type == CRLF)
		throw BadSyntax(PING, ERR_NOORIGIN);
}

// NumericReply	*PingPongParam::mapSyntaxErrorToNumeric(int errCode, NumericReplyFactory replyFactory) {
// 	 switch (errCode) {
// 		case ERR_NOORIGIN:
// 			return replyFactory.makeErrNoOrigin(this);
// 		case ERR_GENERIC:
// 			return replyFactory.makeErrUnknownCommand();
// 		default:
// 			return (NULL);
// 	}
// }