#pragma once

#include "LexerMessage.hpp"
#include "Message.hpp"

class NumericReply;  // ← FORWARD DECLARATION

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
		virtual NumericReply	mapSyntaxErrorToNumeric(int errorCode) const = 0;
		virtual void			validateParam() = 0; //Si algun error sintactico, esto lanza excep.
};

class NickParam: public Param {
	
	public:
		std::string nickname;

		enum SyntaxError {
			ERR_ERRONEUSNICKNAME,
			ERR_NONICKNAMEGIVEN,
			ERR_GENERIC
		};
		NickParam(msgTokens tokens);
		~NickParam() {}
		NickParam& operator=(const NickParam& other) {
			if (this != &other)
				nickname = other.nickname;
			return (*this);
		}
};

class UserParam: public Param {
	std::string username;
	std::string usermode;
	std::string unused;
	std::string realname;

	enum SyntaxError {
		ERR_NEEDMOREPARAMS,
		ERR_GENERIC
	};

	public:
		UserParam(msgTokens tokens);
		~UserParam() {}
		UserParam& operator=(const UserParam& other) {
			if (this != &other)
				username = other.username;
				realname = other.realname;
			return (*this);
		}
};

class PassParam: public Param {
	std::string password; //La contraseña tiene que ser == server.pwI

	enum SyntaxError {
		ERR_NEEDMOREPARAMS,
		ERR_GENERIC
	};

	public:
		PassParam(msgTokens tokens);
		~PassParam() {}
		PassParam& operator=(const PassParam& other) {
			return (*this);
		}
};

class PingParam: public Param {
	std::string password; //La contraseña tiene que ser == server.pwI

	enum SyntaxError {
		ERR_NOORIGIN,
		ERR_GENERIC
	};

	public:
		PingParam(msgTokens tokens);
		~PingParam() {}
		PingParam& operator=(const PingParam& other) {
			return (*this);
		}
};