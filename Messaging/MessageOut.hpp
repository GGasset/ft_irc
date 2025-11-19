#pragma once

#include "Message.hpp"

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
		void	get_ids_from_users(std::vector<User&> uvec) {
			ids.clear();
			for (int i = 0; i < uvec.size(); i++)
				ids.push_back(uvec[i].get_id());
		}
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
				std::vector<size_t> channel_users;
				UsersTarget u(server, channel_users);
				u.get_ids_from_users(server.get_channel_users(ch));
				u.deliver(msg);
			}
		}

};

class MessageOut
{
	/* 	std::string prefix; //esto sirve tanto para rpl como para forwarding
		std::string command; //esto sirve para numeric como para forwarding 
	*/
	char		msg[512]; //El mensaje que se serializa para mandar a GG.
	Param		*param; // unique_ptr<Param>   *params;
	std::unique_ptr<MessageTarget>	*target; //target envía indistintamente para canales como para usuarios.
	virtual void	serialize() = 0;
	virtual void	fill_prefix() = 0; //Servername ó n!u@h

	public:
		MessageOut() {}
		MessageOut(Param *param): param(param) {}
		MessageOut	operator=(const MessageOut& other);
		virtual	~MessageOut() = 0;
		void	*get_msg();
		void	setTarget(std::unique_ptr<MessageTarget> target);
};

class NumericReply: virtual public MessageOut {
	unsigned int code; //Para no tener que comprobar si es alfanumerico. Solo se comprueba rango 001-552

	public:
		NumericReply(unsigned int code, Param *param): MessageOut(param),
													   code(code) {}

};

class ForwardedCommand: virtual public MessageOut {
	Param	*param;

	public:
		ForwardedCommand(Param *param): MessageOut(param) {}

};