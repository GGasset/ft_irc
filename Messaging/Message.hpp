#ifndef MESSAGE_H
# define MESSAGE_H 

# include <iostream>
# include <string>
# include <string_view>
# include <vector>
# include <cassert>
# include <string.h>
# include <ctype.h>
# include <assert.h>
# include "Param.hpp"
# include "Server.hpp"

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

/* Clase que guarda la información del mensaje del cliente. */
class MessageIn {
	COMMAND cmd;
	// unique_ptr<Param>   *params;
	Param   *params;
	size_t	client_id; //id del cliente que envia el mensaje

	public:
		MessageIn(): cmd(COMMAND0) {}
		MessageIn& operator=(const MessageIn& other) {
			if (this != &other) {
				cmd = other.cmd;
				client_id = other.client_id;
				// params = other.params; Estoy hay que verlo.
			}
			return (*this);
		}

		COMMAND	getCommand() {return (cmd);}
		void	setCommand(COMMAND command) {cmd = command;}
		msgTokens tokens;
};

class fnHandlers
{
	MessageOut (*fun[COMMAND0])(size_t, MessageIn, Server&);

	public:
		fnHandlers();
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
extern const std::vector<MessageOut> g_Handler_Queue;

/* Tokenizador */
msgs getMsgs(std::string packet);
std::string getSPACE(std::string &packet, size_t &beginSpace);
std::string getTRAIL(std::string &packet, size_t &beginWord);
std::string getWORD(std::string &packet, size_t &beginWord);
bool isNUMBER(const std::string &param);
char iterStr(const std::string& str);
msgTokens msgTokenizer(std::string msg);
void newSPACE(msgTokens &ret, std::string &msg, size_t &begin);

/* Parseo */
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

/* Utilidades de Manejadores */
void    	complete_registry(User user);
MessageOut  sendNumeric(const std::string user, size_t num);
msgTokens   getPARAMS(msgTokens tokens);

/* Manejadores */
MessageOut handleNick(size_t clientId, MessageIn in, Server &server);

#endif