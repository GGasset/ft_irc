#pragma once

#include "Message.hpp"

enum ReplyCode {
    // 001–004: Respuestas de bienvenida
    RPL_WELCOME               = 001,
    RPL_YOURHOST              = 002,
    RPL_CREATED               = 003,
    RPL_MYINFO                = 004,

    // PASS / USER / NICK
    ERR_NEEDMOREPARAMS        = 461,  // <nick> <cmd> :Not enough parameters
    ERR_ALREADYREGISTRED       = 462,  // <nick> :Unauthorized command (already registered)
    ERR_NONICKNAMEGIVEN       = 431,  // <nick> :No nickname given
    ERR_ERRONEUSNICKNAME      = 432,  // <nick> <nick> :Erroneous nickname
    ERR_NICKNAMEINUSE         = 433,  // <nick> <nick> :Nickname is already in use
    ERR_UNAVAILRESOURCE       = 437,  // <nick> <nick> :Nick/channel temporarily unavailable
    ERR_RESTRICTED            = 484,  // <nick> :Your connection is restricted!

    // JOIN
    RPL_NOTOPIC               = 331,  // <nick> <channel> :No topic is set
    RPL_TOPIC                 = 332,  // <nick> <channel> :<topic>

    ERR_NOSUCHCHANNEL         = 403,  // <nick> <channel> :No such channel
    ERR_TOOMANYCHANNELS       = 405,  // <nick> <channel> :Too many channels
    ERR_CHANNELISFULL         = 471,  // <nick> <channel> :Cannot join (+l)
    ERR_INVITEONLYCHAN        = 473,  // <nick> <channel> :Cannot join (+i)
    ERR_BANNEDFROMCHAN        = 474,  // <nick> <channel> :Cannot join (+b)
    ERR_BADCHANNELKEY         = 475,  // <nick> <channel> :Cannot join (+k)
    ERR_BADCHANMASK           = 476,  // <nick> <channel> :Bad channel mask
    ERR_CHANOPRIVSNEEDED      = 482,  // <nick> <channel> :You're not channel operator
    ERR_UNSUPPORTEDCHANMODE   = 477,  // <nick> <channel> :Channel doesn't support modes

    // PART
    ERR_NOTONCHANNEL          = 442,  // <nick> <channel> :You're not on that channel

    // PRIVMSG
    ERR_NORECIPIENT           = 411,  // <nick> :No recipient given (PRIVMSG)
    ERR_NOTEXTTOSEND          = 412,  // <nick> :No text to send
    ERR_NOSUCHNICK            = 401,  // <nick> <target> :No such nick/channel
    ERR_CANNOTSENDTOCHAN      = 404,  // <nick> <channel> :Cannot send to channel
    ERR_TOOMANYTARGETS        = 407,  // <nick> <target> :Too many targets
    ERR_NOTOPLEVEL            = 413,  // <nick> <mask> :No toplevel domain specified
    ERR_WILDTOPLEVEL          = 414,  // <nick> <mask> :Wildcard in toplevel domain
    ERR_NOORIGIN              = 409,  // <nick> :No origin specified

    // NAMES (tras JOIN o comando NAMES)
    RPL_NAMREPLY              = 353,  // <nick> = <channel> :<names>
    RPL_ENDOFNAMES            = 366,  // <nick> <channel> :End of NAMES list

    // MODE
    RPL_CHANNELMODEIS         = 324,  // <nick> <channel> <modes> <params>
    ERR_UNKNOWNMODE           = 472,  // <nick> <char> :is unknown mode character
    ERR_UMODEUNKNOWNFLAG      = 501,  // <nick> :Unknown MODE flag
    ERR_KEYSET                = 467,  // <nick> <channel> :Channel key already set

    // INVITE
    RPL_INVITING              = 341,  // <nick> <channel> <user>

    ERR_USERNOTINCHANNEL      = 441,  // <nick> <user> <channel> :They aren't on this channel

    // ERR_NOTONCHANNEL (ya definido)
    ERR_USERONCHANNEL         = 443,  // <nick> <user> <channel> :Is already on channel

    // PING
    ERR_NOSUCHSERVER          = 402,  // <nick> <server> :No such server
    
	// COMANDO DESCONOCIDO
    ERR_UNKNOWNCOMMAND        = 421   // <nick> <cmd> :Unknown command
};



class Param {
	protected:
		COMMAND		cmd;
		msgTokens	tokens;

	public:
		
		Param(COMMAND cmd, msgTokens tokens): cmd(cmd), tokens(tokens) {}
		virtual ~Param() = 0;
		COMMAND command() const {return cmd;}

		class BadSyntax: public std::exception {
			COMMAND cmd;
			int		errCode;
			
			public:
				BadSyntax(COMMAND cmd, int errCode): cmd(cmd), errCode(errCode) {}
				int	getErrCode() {return (errCode);}
		};
		// virtual NumericReply	mapSyntaxErrorToNumeric(int errorCode) const = 0;
		virtual void			validateParam() = 0; //Si algun error sintactico, esto lanza excep.
};

class NickParam: public Param {
	
	public:
		std::string nickname;

		NickParam(msgTokens tokens);
		~NickParam() {}
		NickParam& operator=(const NickParam& other) {
			if (this != &other)
				nickname = other.nickname;
			return (*this);
		}
		virtual void	validateParam();
};

class UserParam: public Param {
	public:
		std::string username;
		std::string usermode;
		std::string unused;
		std::string realname;

		UserParam(msgTokens tokens);
		~UserParam() {}
		UserParam& operator=(const UserParam& other) {
			if (this != &other)
				username = other.username;
				realname = other.realname;
			return (*this);
		}
		virtual void	validateParam();
};

class PassParam: public Param {
	public:
		std::string password; //La contraseña tiene que ser == server.pwI

		PassParam(msgTokens tokens);
		~PassParam() {}
		PassParam& operator=(const PassParam& other) {
			return (*this);
		}
		virtual void	validateParam();
};

/* 
Vale, pero como la comunicación entre servidores
no hay que implementarla, 
yo tengo que buscar solo hosts clientes.
Si se relaciona con el hostname de algun cliente guardado
en el estado entonces devuelvo PONG server,
en caso contrario devuelvo el error 402. 
En caso de que se indique server1 y server2, 
trabajo como acabo de decir, 
en caso de que solo este server1,
como es un mensaje para mi y
no un mensaje que tenga que pasar a otro servidor/cliente
 respondo con un PONG <a> y tan pancho, verdad?
*/
class PingPongParam: public Param {
	public:
		std::string server1; //El mensaje si no hay server2, el origen si hay server2
		std::string server2 = "";

		PingPongParam(msgTokens tokens);
		~PingPongParam() {}
		PingPongParam& operator=(const PingPongParam& other) {
			return (*this);
		}
		virtual void	validateParam();
};

Param	*ParamsFactory(COMMAND cmd, msgTokens tokens);