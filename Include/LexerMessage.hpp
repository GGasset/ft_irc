#ifndef LEXERMESSAGE_H
# define LEXERMESSAGE_H

# include <vector>
# include <string>

typedef std::vector<std::string> msgs;

# define NPOS 18446744073709551615UL

enum msg_token_type
{
	PREFIX,
	SPACE,
	WORD,
	NUMBER,
	TOK_PARAM,
	COMMA_LIST,
	TRAIL,
	CRLF
};

typedef struct msg_token
{
	msg_token_type type;
	std::string str;
}	msg_token;

typedef std::vector<msg_token> msgTokens;

enum msgState
{
	PRIX = 0,
	CMD,
	PAR
};

enum COMMAND {
	NICK,
	USER,
	PING,
	PONG,
	PASS,
	QUIT,
	JOIN,
	PART,
	PRIVMSG,
	MODE,
	TOPIC,
	INVITE,
	KICK,
	NOTICE,
	NAMES,
	WHO,
	WHOIS,
	COMMAND0
};

#endif