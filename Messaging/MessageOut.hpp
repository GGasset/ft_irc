#pragma once

#include "Message.hpp"
#include <sstream>
#include <iomanip>

class MessageTarget {
	protected:
		Server &server;
		std::vector<size_t>	ids; //Esto sirve para tanto chanels como usuarios.

	public:
		MessageTarget(Server& server, std::vector<size_t> ids): server(server), ids(ids) {}
		virtual ~MessageTarget() = 0;
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
		// void	get_ids_from_users(std::vector<User&> uvec) {
		// 	ids.clear();
		// 	for (int i = 0; i < uvec.size(); i++)
		// 		ids.push_back(uvec[i].get_id());
		// }
		void	deliver(void *msg) {
			if (!msg)
				return ;
			for (int i = 0; i < ids.size(); i++)
				server.add_msg(msg, 512, false, server.getUsers()[ids[i]]);
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
			for (int i = 0; i < ids.size(); i++) {
				// ch = server.get_channel_by_id(ids[i]);
				// std::vector<size_t> channel_users;
				// UsersTarget u(server, channel_users);
				// u.get_ids_from_users(server.get_channel_users(ch)); Menuedo cipote.
				// u.deliver(msg);
			}
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
		virtual void	serialize() = 0;
		virtual void	fill_prefix() = 0; //Servername ó n!u@h

	public:
		size_t	sender_id; //id del cliente que envia el mensaje
		MessageOut(Server &server): server(server) {}
		// MessageOut	&operator=(const MessageOut& other);
		virtual	~MessageOut() = 0;
		void	*get_msg();
		void	setTarget(MessageTarget *target);
		void	deliver() {target->deliver(msg);}
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
			rpl_msg = prefix +  " " + codetoa() + rpl_msg;
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
};

class ErrErroneusNickname: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Erroneous nickname";
	}

	public:
		ErrErroneusNickname(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, 1),
															  np(param) {}
		~ErrErroneusNickname() {}
};

class ErrNoNicknamegiven: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		rpl_msg = ":No nickname given";
	}
	public:
		ErrNoNicknamegiven(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, 1),
															  np(param) {}
		~ErrNoNicknamegiven() {}
};

class ForwardedCommand: virtual public MessageOut {
	Param	*param;

	void	fill_prefix() {
		User u = server.getUsers()[sender_id];
		prefix = u.get_nick() + "!" + u.getUsername() + "@"; //+ u.getHostname;
	}

	public:
		ForwardedCommand(Server &server, Param *param): MessageOut(server) {}

};