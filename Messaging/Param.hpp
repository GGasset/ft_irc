#pragma once

#include "Message.hpp"

enum ReplyCode {
    // 001–004: Respuestas de bienvenida
    RPL_WELCOME               = 001,
    RPL_YOURHOST              = 002,
    RPL_CREATED               = 003,
    RPL_MYINFO                = 004,

	ERR_GENERIC				  = 0,

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
    ERR_UNKNOWNCOMMAND        = 421,   // <nick> <cmd> :Unknown command

    //WHO
    RPL_WHOREPLY              = 352,

    // WHOIS
    RPL_WHOISUSER             = 311,
    RPL_WHOISERVER            = 312,

};

class Param {
	protected:
		COMMAND		cmd;
		msgTokens	tokens;

	public:
		
		Param(COMMAND cmd): cmd(cmd) {}
		Param(COMMAND cmd, msgTokens tokens): cmd(cmd), tokens(tokens) {}
		virtual ~Param() = 0;
		COMMAND command() const {return cmd;}

		class BadSyntax: public std::exception {
			COMMAND 	cmd;
			ReplyCode	errCode;
			
			public:
				BadSyntax(COMMAND cmd, ReplyCode errCode): cmd(cmd), errCode(errCode) {}
				ReplyCode	getErrCode() {return (errCode);}
				COMMAND		getCmd() {return (cmd);}
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
		std::string	nickname;
		std::string	hostname;

		UserParam();
		UserParam(msgTokens tokens);
		~UserParam() {}
		UserParam& operator=(const UserParam& other) {
			if (this != &other)
				username = other.username;
				realname = other.realname;
			return (*this);
		}
		UserParam		*operator()(const User &u);
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

class QuitParam: public Param {
	public:
		std::string quit_msg;

		QuitParam(msgTokens tokens);
		~QuitParam() {}
		QuitParam&	operator=(const QuitParam& other) {
			return (*this);
		}
		virtual void	validateParam();

};

class JoinParam : public Param {
public:
    std::vector<std::string> channels;
    std::vector<std::string> keys;

    JoinParam(msgTokens tokens): Param(JOIN, tokens) {}
    ~JoinParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(JOIN, ERR_NEEDMOREPARAMS);

        // Channels (may be comma-separated)
        std::string chanList = tokens[i++].str;
        splitByComma(chanList, channels);

        for (size_t j=0; j<channels.size(); ++j) {
            if (!isValidChannelName(channels[j]))
                throw BadSyntax(JOIN, ERR_BADCHANMASK);
        }

        // Optional keys
        if (tokens[i].type == TOK_PARAM) {
            std::string keyList = tokens[i].str;
            splitByComma(keyList, keys);
        }
    }
};

class PartParam : public Param {
public:
    std::vector<std::string> channels;
    std::string partMsg;

    PartParam(msgTokens tokens): Param(PART, tokens) {}
    ~PartParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(PART, ERR_NEEDMOREPARAMS);

        std::string chanList = tokens[i++].str;
        splitByComma(chanList, channels);

        if (tokens[i].type == TOK_PARAM)
            partMsg = tokens[i].str; // trailing
    }
};

class PrivMsgParam : public Param {
public:
    std::string target;
    std::string text;

    PrivMsgParam(msgTokens tokens): Param(PRIVMSG, tokens) {}
    ~PrivMsgParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(PRIVMSG, ERR_NORECIPIENT);

        target = tokens[i++].str;

        if (tokens[i].type != TOK_PARAM)
            throw BadSyntax(PRIVMSG, ERR_NOTEXTTOSEND);

        text = tokens[i].str;
    }
};

class NoticeParam : public Param {
public:
    std::string target;
    std::string text;

    NoticeParam(msgTokens tokens): Param(NOTICE, tokens) {}
    ~NoticeParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;
        if (tokens[i].type == CRLF)
            return;

        target = tokens[i++].str;
        if (tokens[i].type == TOK_PARAM)
            text = tokens[i].str;
    }
};

class TopicParam : public Param {
public:
    std::string channel;
    std::string topic;

    TopicParam(msgTokens tokens): Param(TOPIC, tokens) {}
    ~TopicParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(TOPIC, ERR_NEEDMOREPARAMS);

        channel = tokens[i++].str;

        if (tokens[i].type == TOK_PARAM)
            topic = tokens[i].str;
    }
};

class InviteParam : public Param {
public:
    std::string nick;
    std::string channel;

    InviteParam(msgTokens tokens): Param(INVITE, tokens) {}
    ~InviteParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(INVITE, ERR_NEEDMOREPARAMS);

        nick = tokens[i++].str;

        if (tokens[i].type != TOK_PARAM)
            throw BadSyntax(INVITE, ERR_NEEDMOREPARAMS);

        channel = tokens[i].str;
    }
};

class KickParam : public Param {
public:
    std::string channel;
    std::string user;
    std::string comment;

    KickParam(msgTokens tokens): Param(KICK, tokens) {}
    ~KickParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(KICK, ERR_NEEDMOREPARAMS);

        channel = tokens[i++].str;

        if (tokens[i].type != TOK_PARAM)
            throw BadSyntax(KICK, ERR_NEEDMOREPARAMS);

        user = tokens[i++].str;

        if (tokens[i].type == TOK_PARAM)
            comment = tokens[i].str;
    }
};

class ModeParam : public Param {
public:
    std::string channel;
    std::string modeStr;
    std::string modeArg;  // opcional, a veces nick o key

    ModeParam(msgTokens tokens): Param(MODE, tokens) {}
    ~ModeParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM)
            i++;

        if (tokens[i].type == CRLF)
            throw BadSyntax(MODE, ERR_NEEDMOREPARAMS);

        channel = tokens[i++].str;

        if (tokens[i].type == CRLF)
            return; // MODE <channel> → listar modos

        modeStr = tokens[i++].str;

        if (tokens[i].type == TOK_PARAM)
            modeArg = tokens[i].str;
    }
};

class NamesParam : public Param {
public:
    std::vector<std::string> channels;   // Lista de canales a consultar
    bool listAll;                        // TRUE si no se ha especificado ningún canal

    NamesParam(msgTokens tokens): Param(NAMES, tokens) {}
    ~NamesParam() {}

    virtual void validateParam() {
        int i = 0;
        while (tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
            i++;
        if (tokens[i].type == CRLF) {
            listAll = true;
            return;
        }
        std::string chanList = tokens[i].str;
        splitByComma(chanList, channels);
        for (size_t j = 0; j < channels.size(); ++j) {
            if (!isValidChannelName(channels[j]))
                throw BadSyntax(NAMES, ERR_BADCHANMASK);
        }
        listAll = false; //No se que tiene que hacer en caso de que falle el hijo de puta.
    }
};

class WhoisParam: public Param {
    public:
        std::string target = ""; //En princpio me la trae floja.
        std::vector<std::string> nicks;
        
        WhoisParam(msgTokens tokens): Param(WHOIS, tokens) {}
        ~WhoisParam() {}

        virtual void    validateParam() {
            int i = 0;

            while (tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
                i++;
            if (tokens[i].type == TOK_PARAM)
            {
                target = tokens[i++].str;
                throw BadSyntax(NAMES, ERR_NOSUCHSERVER);
            }
            if (tokens[++i].type == CRLF)
                throw BadSyntax(NAMES, ERR_NONICKNAMEGIVEN);
            splitByComma(tokens[i].str, nicks);
        }
};

/* Si sale o detras de una máscara, entonces se buscan los que sean operadores. Como no tenemos operadores de servidor, entonces ha de devolver nosuchserver. */
class WhoParam: public Param {
    public:
        std::string mask; //Cualquier cosa.


        WhoParam(msgTokens tokens): Param(WHO, tokens), mask("") {}
        ~WhoParam() {}

        virtual void    validateParam() {
            int i = 0;

            while (tokens[i].type != TOK_PARAM && tokens[i].type != CRLF)
                i++;
            if (tokens[i].type != CRLF)
                mask = tokens[i++].str;
            if (tokens[i].str == "o")
                throw BadSyntax(WHO, ERR_NOSUCHSERVER);
        }
};

Param	*ParamsFactory(COMMAND cmd, msgTokens tokens);