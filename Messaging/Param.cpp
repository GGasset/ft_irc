#include "Param.hpp"

Param::~Param() {}

NickParam::NickParam(msgTokens tokens): Param(NICK, tokens) {}

void	NickParam::validateParam() {
	int i = 0;
	while (tokens[i].type != TOK_PARAM)
		i++;
	if (tokens[i].type == CRLF)
		throw BadSyntax(NICK, ERR_NONICKNAMEGIVEN);
	nickname = tokens[i].str;
	if (!isValidNickName(tokens[i].str))
		throw BadSyntax(NICK, ERR_ERRONEUSNICKNAME);
}

UserParam::UserParam(msgTokens tokens): Param(USER, tokens) {}

void	UserParam::validateParam() {
	// Lo que he extraído del RFC es que se traga lo que sea.
	// Solo se gestiona el caso cuando username es incorrecto.
	int i = 0;
	
	while (tokens[i].type != TOK_PARAM)
		i++;
	tokens[i].type != CRLF ? username = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? usermode = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? unused = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	i++;
	tokens[i].type != CRLF ? realname = tokens[i++].str : throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
	//usermode y unused me la sudan.
	for (i = 0; i < username.size(); i++) {
		if (!isUserChar(username[i]))
			throw BadSyntax(USER, ERR_UNKNOWNCOMMAND);
	}
}

PassParam::PassParam(msgTokens tokens): Param(PASS, tokens) {}

void	PassParam::validateParam() {
	int i = 0;

	while (tokens[i].type != TOK_PARAM)
		i++;
	password = tokens[i].str;
	if (password.empty())
		throw BadSyntax(PASS, ERR_UNKNOWNCOMMAND);
}

PingPongParam::PingPongParam(msgTokens tokens): Param(PING, tokens) {}

void	PingPongParam::validateParam() {
	int i = 0;

	while (tokens[i].type != TOK_PARAM)
		i++;
	if (tokens[i].type == CRLF)
		throw BadSyntax(PING, ERR_NOORIGIN);
}

Param	*ParamsFactory(COMMAND cmd, msgTokens tokens) {
	switch (cmd)
	{
		case NICK :
			return new NickParam(tokens);
		case USER:
			return new UserParam(tokens);
		case PASS:
			return new PassParam(tokens);
		case PING:
			return new PingPongParam(tokens);
		case PONG:
			return new PingPongParam(tokens);
		default:
			NULL;
	}
	return NULL;
}