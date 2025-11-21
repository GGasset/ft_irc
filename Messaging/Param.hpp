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
			ERR_NONICKNAMEGIVEN
		};
		NickParam(msgTokens tokens);
		~NickParam() {}
		NickParam& operator=(const NickParam& other) {
			if (this != &other)
				nickname = other.nickname;
			return (*this);
		}
};

// class UserParam: public Param {
// 	std::string nickname;
// 	std::string realname;
// 	std::string username;
// 	std::string hostname;
// 	// std::string servername;

// 	public:
// 		UserParam(msgTokens tokens);
// 		~UserParam() {}
// 		UserParam& operator=(const UserParam& other) {
// 			if (this != &other)
// 				nickname = other.nickname;
// 				username = other.username;
// 				realname = other.realname;
// 				hostname = other.hostname;
// 			return (*this);
// 		}
// };