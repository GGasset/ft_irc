#include "Param.hpp"
#include <unistd.h>

Param::~Param() {}

NickParam::NickParam(msgTokens tokens): Param(NICK, tokens) {}

void	NickParam::validateParam() {
	int i = 0;
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(NICK, ERR_NONICKNAMEGIVEN);
	nickname = tokens[i].str;
	if (!isValidNickName(tokens[i].str))
		throw BadSyntax(NICK, ERR_ERRONEUSNICKNAME);
}


UserParam::UserParam(): Param(USER) {}

UserParam::UserParam(msgTokens tokens): Param(USER, tokens) {}

UserParam	*UserParam::operator()(const User &u) {
	nickname = u.get_nick();
	username = u.getUsername();
	realname = u.getRealname();
	hostname = u.getHostname();
	return (this);
}

void	UserParam::validateParam() {
	// Lo que he extraído del RFC es que se traga lo que sea.
	// Solo se gestiona el caso cuando username es incorrecto.
	int i = 0;
	
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (tokens.size() - i < 7) throw BadSyntax(USER, ERR_NEEDMOREPARAMS);
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
			throw BadSyntax(USER, ERR_GENERIC);
	}
}


PassParam::PassParam(msgTokens tokens): Param(PASS, tokens) {}

void	PassParam::validateParam() {
	int i = 0;

	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	password = tokens[i].str;
	if (password.empty())
		throw BadSyntax(PASS, ERR_NEEDMOREPARAMS);
}

PingPongParam::PingPongParam(msgTokens tokens, COMMAND cmd): Param(cmd, tokens) {}

void	PingPongParam::validateParam() {
	int i = 0;

	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size())
	{
		throw BadSyntax(PING, ERR_NOORIGIN);
	}
	server1 = tokens[i++].str;
	if (tokens[i].type == TOK_PARAM)
		server2 = tokens[i].str;
}

PingPongParam::PingPongParam(std::string server1): Param(PONG, std::vector<msg_token>{}),
												   server1(server1) {}

QuitParam::QuitParam(msgTokens tokens): Param(QUIT, tokens) {}

void	QuitParam::validateParam() {}

JoinParam::JoinParam(msgTokens tokens): Param(JOIN, tokens) {}

void	JoinParam::validateParam()
{
    int i = 0;
    while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
        i++;
    if (i >= (int)tokens.size() || tokens[i].type == CRLF)
            throw BadSyntax(JOIN, ERR_NEEDMOREPARAMS);
    std::string chanList = tokens[i++].str;
    splitByComma(chanList, channels);
    for (size_t j=0; j<channels.size(); ++j)
	{
        if (!isValidChannelName(channels[j]))
            throw BadSyntax(JOIN, ERR_BADCHANMASK);
    }
    if (tokens[i].type == TOK_PARAM)
	{
        std::string keyList = tokens[i].str;
        splitByComma(keyList, keys);
    }
}
PartParam::PartParam(msgTokens tokens): Param(PART, tokens) {}

void	PartParam::validateParam()
{
    int i = 0;
    while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
        i++;
    if (i >= (int)tokens.size() || tokens[i].type == CRLF)
        throw BadSyntax(PART, ERR_NEEDMOREPARAMS);
    std::string chanList = tokens[i++].str;
    splitByComma(chanList, channels);
    if (tokens[i].type == TOK_PARAM)
        partMsg = tokens[i].str; // trailing
}

PrivMsgParam::PrivMsgParam(msgTokens tokens): Param(PRIVMSG, tokens) {}

void PrivMsgParam::validateParam()
{
    int i = 0;
    while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
        i++;
    if (i >= (int)tokens.size() || tokens[i].type == CRLF)
        throw BadSyntax(PRIVMSG, ERR_NORECIPIENT);
    target = tokens[i++].str;
    if (tokens[i].type != TOK_PARAM)
        throw BadSyntax(PRIVMSG, ERR_NOTEXTTOSEND);
    text = tokens[i].str;
}

NoticeParam::NoticeParam(msgTokens tokens): Param(NOTICE, tokens) {}

void	NoticeParam::validateParam()
{
        int i = 0;
        while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
            i++;
        if (i >= (int)tokens.size() || tokens[i].type == CRLF)
            return;
        target = tokens[i++].str;
        if (tokens[i].type == TOK_PARAM)
            text = tokens[i].str;
}

TopicParam::TopicParam(msgTokens tokens): Param(TOPIC, tokens) {}

void TopicParam::validateParam()
{
	int i = 0;
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(TOPIC, ERR_NEEDMOREPARAMS);
	channel = tokens[i++].str;
	if (tokens[i].type == TOK_PARAM)
		topic = tokens[i].str;
}

InviteParam::InviteParam(msgTokens tokens): Param(INVITE, tokens) {}

void	InviteParam::validateParam()
{
	int i = 0;
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(INVITE, ERR_NEEDMOREPARAMS);
	nick = tokens[i++].str;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(INVITE, ERR_NEEDMOREPARAMS);
	channel = tokens[i].str;
}

KickParam::KickParam(msgTokens tokens): Param(KICK, tokens) {}

void 	KickParam::validateParam()
{
	int i = 0;
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(KICK, ERR_NEEDMOREPARAMS);
	channel = tokens[i++].str;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(KICK, ERR_NEEDMOREPARAMS);
	user = tokens[i++].str;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(KICK, ERR_NEEDMOREPARAMS);
	comment = tokens[i].str;
}

ModeParam::ModeParam(msgTokens tokens): Param(MODE, tokens) {}

void	ModeParam::validateParam()
{
	int i = 0;
	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
		i++;
	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
		throw BadSyntax(MODE, ERR_NEEDMOREPARAMS);
	channel = tokens[i++].str;
	if (tokens[i].type == CRLF)
		return; // MODE <channel> → listar modos
	modeStr = tokens[i++].str;
	if (tokens[i].type == TOK_PARAM)
		modeArg = tokens[i].str;
}

// WhoisParam::WhoisParam(msgTokens tokens): Param(WHOIS, tokens) {}

// void	WhoisParam::validateParam()
// {
// 	int i = 0;
// 	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
// 		i++;
// 	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
// 		throw BadSyntax(WHOIS, ERR_NONICKNAMEGIVEN);
// 	nicks.push_back(tokens[i++].str);
// 	if (tokens[i].type == CRLF)
// 		return; // WHOIS <nick> → listar modos
// }

// WhoParam::WhoParam(msgTokens tokens): Param(WHO, tokens), mask("") {}

// void WhoParam::validateParam()
// {
// 	int i = 0;
// 	while (i < (int)tokens.size() && tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
// 		i++;
// 	if (i >= (int)tokens.size() || tokens[i].type == CRLF)
// 		throw BadSyntax(WHO, ERR_NONICKNAMEGIVEN);
// 	mask = tokens[i++].str;
// 	if (tokens[i].str == "o")
// 		throw BadSyntax(WHO, ERR_NOSUCHSERVER);
// }

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
			return new PingPongParam(tokens, cmd);
		case PONG:
			return new PingPongParam(tokens, cmd);
		case QUIT:
			return new QuitParam(tokens);
		case JOIN:
			return new JoinParam(tokens);
		case PART:
			return new PartParam(tokens);
		case PRIVMSG:
			return new PrivMsgParam(tokens);
		case NOTICE:
			return new NoticeParam(tokens);
		case TOPIC:
			return new TopicParam(tokens);
		case INVITE:
			return new InviteParam(tokens);
		case KICK:
			return new KickParam(tokens);
		case MODE:
			return new ModeParam(tokens);
		// case NAMES:
		// 	return new NamesParam(tokens);
		default:
			NULL;
	}
	return NULL;
}


