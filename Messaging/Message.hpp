#ifndef MESSAGE_H
# define MESSAGE_H 

# include <string>
# include <string_view>
# include <vector>
# include <cassert>
# include <iostream>
# include <assert.h>

typedef std::vector<std::string> msgs;

# define NPOS 18446744073709551615UL

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

enum ParseStatus {
    VALID_MSG,
	PERR_MSG_LENGTH,
	PERR_NO_CRLF,
	PERR_NO_SPACE_AFTER_PREFIX,
	PERR_PREFIX_LENGTH,
	PERR_PREFIX_MISSING_NICK,
	PERR_PREFIX_MISSING_USER,
	PERR_PREFIX_MISSING_HOST,
	PERR_PREFIX_INVALID_SERVERNAME,
	PERR_INVALID_COMMAND,
	PERR_NUMERIC_COMMAND_TOO_LONG,
	PERR_INVALID_CHARACTERS,
	PERR_EMPTY_SPACE,
	PERR_EXCEED_15_PARAM,
	PERR_NONE // must always be the last one
};

// Please, make message a POD type 😵🤙
typedef struct MessageOut
{
    char    msg[512];
    void    fillMsgOut();
}   MessageOut;


bool		isNUMBER(const std::string &param);
MessageIn   parseMessage(msgTokens tokens, ParseStatus &status);

#endif