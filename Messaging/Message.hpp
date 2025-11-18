#ifndef MESSAGE_H
# define MESSAGE_H 

# include <string>
# include "string.h"
# include <string_view>
# include <vector>
# include <cassert>
# include <ctype.h>
# include <iostream>
# include "Server.hpp"
# include <assert.h>

typedef std::vector<std::string> msgs;

# define NPOS 18446744073709551615UL

enum msg_token_type
{
	PREFIX,
	SPACE,
	WORD,
	NUMBER,
	PARAM,
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
	COMMAND0
};

// Please, make message a POD type 😵🤙
typedef struct MessageOut
{
	std::vector<size_t>	ids; //Esto sirve para tanto chanels como usuarios.
	bool	to_user; //Esto indica si es para un usuario o para un canal.
    char    msg[512];
    void    fillMsgOut(User u, std::string servername, std::string cmd, std::string params);
	void	appendMsgOutQueue(); //Basicamente recorre ids y despues los mete en la cola.
}   MessageOut;

typedef struct  MessageIn
{
	msgTokens	tokens;
	COMMAND		cmd;
}   MessageIn;

class fnHandlers
{
	MessageOut (*fun[COMMAND0])(size_t, MessageIn, Server&);

	public:
		fnHandlers();
		// fnHandlers(const fnHandlers &fn);
		// fnHandlers	&operator=(const fnHandlers &fn);
		~fnHandlers();
		MessageOut	operator()(COMMAND cmd, MessageIn msg, Server& server);
		
};

enum ParseStatus {
    VALID_MSG,
	PERR_MSG_LENGTH,
	PERR_NO_CRLF,
	PERR_NO_SPACE_AFTER_PREFIX,
	PERR_PREFIX_LENGTH,
	PERR_PREFIX_MISSING_NICK,
	PERR_PREFIX_MISSING_USER,
	PERR_PREFIX_MISSING_HOST,
	PERR_PREFIX_INVALID_NICK,
	PERR_PREFIX_INVALID_USER,
	PERR_PREFIX_INVALID_HOST,
	PERR_PREFIX_INVALID_SERVERNAME,
	PERR_MISSING_COMMAND,
	PERR_INVALID_COMMAND,
	PERR_NUMERIC_COMMAND_TOO_LONG,
	PERR_INVALID_CHARACTERS,
	PERR_EMPTY_SPACE,
	PERR_EXCEED_15_PARAM,
	PERR_NONE // must always be the last one
};

extern const std::string g_parseErrors[PERR_NONE];

msgs getMsgs(std::string packet);
std::string getSPACE(std::string &packet, size_t &beginSpace);
std::string getTRAIL(std::string &packet, size_t &beginWord);
std::string getWORD(std::string &packet, size_t &beginWord);
bool isNUMBER(const std::string &param);
char iterStr(const std::string& str);
msgTokens msgTokenizer(std::string msg);
void newSPACE(msgTokens &ret, std::string &msg, size_t &begin);

COMMAND getCMD(const std::string &cmd);
inline bool isCharInSet(char c, const std::string& set);
inline bool isInNospcrlfcl(const std::string& str);
inline bool isInNospcrlfclTRAIL(const std::string& str);
inline bool isReservedChar(char c);
inline bool isSpecialChar(char c);
bool isValidHostName(const std::string &hostaname);
bool isValidNickName(const std::string &nickname); 
bool isValidServerName(const std::string& name);
bool isValidUserName(const std::string &username);


MessageIn   parseMessage(msgTokens tokens, ParseStatus &status);

#endif