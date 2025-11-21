#ifndef MESSAGE_H
# define MESSAGE_H 

# include "Server.hpp"
# include <iostream>
# include <string>
# include <vector>
# include <cassert>
# include <string.h>
# include <ctype.h>
# include <assert.h>
# include <memory>
# include "LexerMessage.hpp"
# include "Param.hpp"
# include "MessageOut.hpp"

/* Clase que guarda la información del mensaje del cliente. */
class MessageIn {
	COMMAND		cmd;
	// std::string prefix; Realmente es util?? Tengo el id del sender, puedo acceder a nick, user, host desde allí.
	Param		*params;

	public:
		MessageIn(): cmd(COMMAND0) {}
		MessageIn(COMMAND cmd): cmd(cmd) {}
								// prefix(prefix) {}
		MessageIn& operator=(const MessageIn& other) {
			if (this != &other) {
				cmd = other.cmd;
				sender_id = other.sender_id;
				// params = other.params; Estoy hay que verlo. Quizá con nuestra propia implementación de shared_ptr.
			}
			return (*this);
		}

		COMMAND	getCommand() {return (cmd);}
		void	setCommand(COMMAND command) {cmd = command;}
		Param	*getParams() { return (params);}
		void	setParams(Param *param) {params = param;}
		msgTokens tokens;
		size_t	sender_id; //id del cliente que envia el mensaje
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
inline bool isUserChar(char c);
bool isValidHostName(const std::string &hostaname);
bool isValidNickName(const std::string &nickname); 
bool isValidServerName(const std::string& name);
bool isValidUserName(const std::string &username);

MessageIn   parseMessage(msgTokens tokens, ParseStatus &status);

/* Utilidades de Manejadores */
void    	complete_registry(User user);
// MessageOut  sendNumeric(const std::string user, size_t num);
msgTokens   getPARAMS(msgTokens tokens);

/* Manejadores */
// MessageOut handleNick(size_t clientId, MessageIn in, Server &server);

#endif