#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <iostream>

typedef std::vector<std::string> msgs;

#define NPOS 18446744073709551615UL

enum msg_token_type
{
	PREFIX,
	SPACE,
	COLON,
	WORD,
	NUMBER,
	COMMA_LIST,
	TRAIL
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
	PARAM
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
	COMMAND0
};


typedef struct  MessageIn
{
	msgTokens	tokens;
	COMMAND		cmd;
}   MessageIn;

// Please, make message a POD type 😵🤙
typedef struct MessageOut
{
    char    msg[512];
    void    fillMsgOut();
}   MessageOut;


bool	isNUMBER(const std::string &param);