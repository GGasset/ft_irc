#pragma once

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
		// virtual NumericReply	mapSyntaxErrorToNumeric(int errorCode) const = 0;
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
		virtual void	validateParam();
};

class UserParam: public Param {
	public:
		std::string username;
		std::string usermode;
		std::string unused;
		std::string realname;

		enum SyntaxError {
			ERR_NEEDMOREPARAMS,
			ERR_GENERIC
		};
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

		enum SyntaxError {
			ERR_NEEDMOREPARAMS,
			ERR_GENERIC
		};
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

		enum SyntaxError {
			ERR_NOORIGIN,
			ERR_GENERIC
		};
		PingPongParam(msgTokens tokens);
		~PingPongParam() {}
		PingPongParam& operator=(const PingPongParam& other) {
			return (*this);
		}
		virtual void	validateParam();
};
