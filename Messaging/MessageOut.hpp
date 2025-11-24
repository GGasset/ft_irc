#pragma once

#include "Param.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

class MessageTarget {
	protected:
		Server &server;
		std::vector<size_t>	ids; //Esto sirve para tanto chanels como usuarios.

	public:
		MessageTarget(Server& server, std::vector<size_t> ids): server(server), ids(ids) {}
		virtual ~MessageTarget() {}
		virtual	void deliver(void *msg) = 0;
};

class UsersTarget : public MessageTarget {
	public:
		UsersTarget(Server& server, std::vector<size_t> ids): MessageTarget(server, ids) {}
		UsersTarget(const UsersTarget& other): MessageTarget(other.server, other.ids) {}
		~UsersTarget() {}
		UsersTarget&	operator=(const UsersTarget& other) {
			if (this != &other)
				ids = other.ids;
			return (*this);
		}
		void	deliver(void *msg) {
			if (!msg)
				return ;
			for (int i = 0; i < ids.size(); i++)
				server.add_msg(msg, 512, false, server.get_user_by_id(ids[i]));
		}
};

class ChannelTarget : public MessageTarget {
	public:
		ChannelTarget(Server& server, std::vector<size_t> ids): MessageTarget(server, ids) {}
		ChannelTarget(const ChannelTarget& other): MessageTarget(other.server, other.ids) {}
		~ChannelTarget() {}
		ChannelTarget&	operator=(const ChannelTarget& other) {
			if (this != &other)
				ids = other.ids;
			return (*this);
		}
		void	deliver(void *msg) {
			Channel ch;

			if (!msg)
				return ;
			// for (int i = 0; i < ids.size(); i++) {
			// 	ch = server.get_by_channel_id(ids[i]);
			// 	server.add_msg(msg, 512, false, ch);
			// }
			ch = server.get_by_channel_id(ids[0]);
			/* Esto esta fatal, es para que no toque los huevos. */
			server.add_msg(msg, 512, false, server.get_user_by_id(ch.get_members()[0]));
		}

};

class MessageTargetFactory {
	public:
		MessageTargetFactory() {}
		static MessageTarget	*create(Server &server, std::vector<size_t> ids, char t) {
			if (t == 'u')
				return new UsersTarget(server, ids);
			else if (t == 'c')
				return new ChannelTarget(server, ids);
			else
				return NULL;
		}
};

class MessageOut
{
	protected:
		Server			&server;
		std::string		prefix;
		char			msg[512]; //El mensaje que se serializa para mandar a GG.
		std::string		rpl_msg;
		MessageTarget	*target; //target envía indistintamente para canales como para usuarios.
		virtual void	assemble_msg() = 0;
		virtual void	fill_prefix() = 0; //Servername ó n!u@h
		virtual void	serialize() = 0;

	public:
		size_t	sender_id; //id del cliente que envia el mensaje
		MessageOut(Server &server): server(server) {}
		// MessageOut	&operator=(const MessageOut& other);
		virtual	~MessageOut() = 0;
		void	*get_msg();
		void	setTarget(MessageTarget *target);
		void	deliver() {target->deliver(msg);}
		std::string getRpl(void) {return rpl_msg;}//Esto solo de testeo
};

class NumericReply: virtual public MessageOut {
	protected:
		unsigned int code; //Para no tener que comprobar si es alfanumerico. Solo se comprueba rango 001-552

		void	fill_prefix() {
			// prefix = server.getServerName(); Nombre del archivo de configuración
			prefix = "irc.local";
		}

		void	serialize() {
			fill_prefix();
			assemble_msg();
			rpl_msg = prefix +  " " + codetoa() + rpl_msg + "\r\n";
			memcpy(msg, rpl_msg.c_str(), rpl_msg.length());
		}

	public:
		NumericReply(Server &server, unsigned int code): MessageOut(server),
													   				code(code) {}
		~NumericReply() {}
		std::string	codetoa() {
			std::ostringstream ss;
			ss << std::setw(3) << std::setfill('0') << code;
			return ss.str();
		}
		unsigned int getCode() {return code;}
};

class NickParam;

class RplWelcome: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		User u = server.getUsers()[sender_id];
		std::string	sender_info = u.get_nick() + "!" + u.getUsername() + "@";
		rpl_msg = "Welcome to the Internet Relay Network " + sender_info;
	}

	public:
		RplWelcome(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_WELCOME),
													  up(param) {}
		~RplWelcome() {}
};

class RplYourHost: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		User u = server.getUsers()[sender_id];
		std::string	servername = "irc.local"; //Esto deberia estar dentro de Server, que viene de la configuracion.
		std::string ver = "1.1"; //Me lo invento. Tambien deberia estar dentro de la configuración.
		rpl_msg = "Your host is " + servername + ", running version " + ver;
	}

	public:
		RplYourHost(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_YOURHOST),
													  up(param) {}
		~RplYourHost() {}
};

class RplCreated: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		std::time_t now = std::time(nullptr);
		std::tm *ltm = std::localtime(&now);

		std::ostringstream oss;
		oss << (1900 + ltm->tm_year) << "-"
			<< std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << "-"
			<< std::setw(2) << std::setfill('0') << ltm->tm_mday;
		rpl_msg = "This server was created " + oss.str();
	}

	public:
		RplCreated(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_CREATED),
													  up(param) {}
		~RplCreated() {}
};

// class RplMyInfo: public NumericReply {

// };

class ErrErroneousNickname: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Erroneous nickname";
	}

	public:
		ErrErroneousNickname(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_ERRONEUSNICKNAME),
															  np(param) {}
		~ErrErroneousNickname() {}
};

class ErrNoNicknamegiven: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		rpl_msg = ":No nickname given";
	}
	public:
		ErrNoNicknamegiven(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_NONICKNAMEGIVEN),
															  np(param) {}
		~ErrNoNicknamegiven() {}
};

class ErrNicknameInUse: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Nickname is already in use";
	}
	public:
		ErrNicknameInUse(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_NICKNAMEINUSE),
															  np(param) {}
		~ErrNicknameInUse() {}
};

class ErrUnavailResource: public NumericReply {
	NickParam 	*np;
	std::string	name;

	void	assemble_msg() {
		rpl_msg = name + " :Nick/channel is temporarily unavailable";
	}
	public:
		ErrUnavailResource(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_UNAVAILRESOURCE),
															  np(param),
															  name(param->nickname) {}
		/* ..... Se ha de overridear para todos los Comandos que lo utilizen.  */
		~ErrUnavailResource() {}
};

class ErrNeedMoreParams: public NumericReply {
	Param 	*p;
	std::string	commandname;

	void	assemble_msg() {
		rpl_msg = commandname + " :Not enough parameters";
	}
	public:
		ErrNeedMoreParams(Server &server, COMMAND commmand): MessageOut(server),
															  NumericReply(server, ERR_NEEDMOREPARAMS),
															  commandname(commandname) {}
		/* ..... Se ha de overridear para todos los Comandos que lo utilizen.  */
		~ErrNeedMoreParams() {}
};

class ErrRestricted: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Nickname is already in use";
	}
	public:
		ErrRestricted(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_RESTRICTED),
															  np(param) {}
		~ErrRestricted() {}
};

class ErrAlredyRegistered: public NumericReply {
	Param	*p;

		void	assemble_msg() {
		rpl_msg = ":Unauthorized command (already registered)";
	}
	public:
		ErrAlredyRegistered(Server &server, UserParam *param): MessageOut(server),
															  NumericReply(server, ERR_ALREADYREGISTRED),
															  p(param) {}
		ErrAlredyRegistered(Server &server, PassParam *param): MessageOut(server),
															   NumericReply(server, ERR_ALREADYREGISTRED),
															   p(param) {}
		~ErrAlredyRegistered() {}
};

class ErrNoOrigin: public NumericReply {
	PingPongParam	*pip;

		void	assemble_msg() {
		rpl_msg = ":No origin specified";
	}
	public:
		ErrNoOrigin(Server &server, PingPongParam *param): MessageOut(server),
													   NumericReply(server, ERR_NOORIGIN),
													   pip(param) {}
		~ErrNoOrigin() {}
};

class ErrNoSuchServer: public NumericReply {
	PingPongParam	*pip;

	void	assemble_msg() {
		rpl_msg = pip->server2 + " :No such server";
	}
	public:
		ErrNoSuchServer(Server &server, PingPongParam *param): MessageOut(server),
													   NumericReply(server, ERR_NOSUCHSERVER),
													   pip(param) {}
		~ErrNoSuchServer() {}
};

class ErrUnknownCommand: public NumericReply {
	void	assemble_msg() {
		rpl_msg = "";
	}
	public:
		ErrUnknownCommand(Server &server): MessageOut(server),
										   NumericReply(server, 0) {}
		~ErrUnknownCommand() {}
};

class NumericReplyFactory {
	Server	&server;
	public:

		static NumericReply *create(ReplyCode code, Server &serv, Param *param);
		NumericReplyFactory(Server &server): server(server) {}

		/* Rpl al registrarse un nuevo usuario */
		static RplWelcome				*makeRplWelcome(Server &serv, UserParam* param) {return new RplWelcome(serv, param);}
		static RplYourHost				*makeRplYourHost(Server &serv, UserParam* param) {return new RplYourHost(serv, param);}
		static RplCreated				*makeRplCreated(Server &serv, UserParam* param) {return new RplCreated(serv, param);}
		// static RplMyInfo				*makeRplMyInfo(Server &serv, UserParam* param) {return new RplMyInfo(serv, param);} Esta falta porque mucha info.


		static ErrErroneousNickname		*makeErrErroneusNickname(Server &serv, NickParam* param) {return new ErrErroneousNickname(serv, param);}
		static ErrNoNicknamegiven		*makeErrNoNicknamegiven(Server &serv, NickParam* param) {return new ErrNoNicknamegiven(serv, param);};
		static ErrNicknameInUse			*makeErrNicknameInUse(Server &serv, NickParam* param) {return new ErrNicknameInUse(serv, param);};
		static ErrUnavailResource		*makeErrUnavailResource(Server &serv, NickParam* param) {return new ErrUnavailResource(serv, param);};
		static ErrRestricted			*makeErrRestricted(Server &serv, NickParam* param) {return new ErrRestricted(serv, param);};
		static ErrNeedMoreParams		*makeErrNeedMoreParams(Server &serv, Param *param) {return new ErrNeedMoreParams(serv, param->command());}
		static ErrAlredyRegistered		*makeErrAlredyRegistered(Server &serv, UserParam *param) {return new ErrAlredyRegistered(serv, param);}
		static ErrNoOrigin				*makeErrNoOrigin(Server &serv, PingPongParam *param) {return new ErrNoOrigin(serv, param);}
		static ErrNoSuchServer			*makeErrNoSuchServer(Server &serv, PingPongParam *param) {return new ErrNoSuchServer(serv, param);}
		static ErrUnknownCommand		*makeErrUnknownCommand(Server &serv) {return new ErrUnknownCommand(serv);}
};

class ForwardedCommand: virtual public MessageOut {
	Param	*param;

	void	fill_prefix() {
		User u = server.getUsers()[sender_id];
		prefix = u.get_nick() + "!" + u.getUsername() + "@"; //+ u.getHostname;
	}
	void	serialize() {
		fill_prefix();
		assemble_msg();
		rpl_msg = prefix +  " " + getCommandname(param->command()) + rpl_msg + "\r\n";
		memcpy(msg, rpl_msg.c_str(), rpl_msg.length());
	}

	public:
		ForwardedCommand(Server &server, Param *param): MessageOut(server), param(param) {}
		~ForwardedCommand() {}
};

class NickForwardedCommand: public ForwardedCommand {
	NickParam	*np;

	public:
		NickForwardedCommand(Server &server, NickParam *param): MessageOut(server),
																ForwardedCommand(server, param),
																np(param) {}
		void	assemble_msg() {
			rpl_msg = "NICK " + np->nickname;
		}
};

class ForwardedCommandFactory {
	Server	&server;
	
	public:
	ForwardedCommandFactory(Server &server): server(server) {}
	static ForwardedCommand	*makeNickForward(Server &serv, NickParam *param) {return new NickForwardedCommand(serv, param);}
	static ForwardedCommand	*create(COMMAND cmd, Server &serv, Param *param);
};