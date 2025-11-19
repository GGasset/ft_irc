#pragma once

#include "Message.hpp"

class MessageTarget {
	Server &server;
	std::vector<size_t>	ids; //Esto sirve para tanto chanels como usuarios.

	public:
		MessageTarget(Server& server, std::vector<size_t> ids): server(server), ids(ids) {}
		virtual ~MessageTarget();
		virtual	void deliver(void *msg);
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
		MessageOut	operator=(const MessageOut& other);
		virtual	~MessageOut() = 0;
		void	*get_msg();
		void	setTarget(std::unique_ptr<MessageTarget> target);
};

class NumericReply: virtual public MessageOut {
	unsigned int code; //Para no tener que comprobar si es alfanumerico. Solo se comprueba rango 001-552

	public:
		NumericReply(unsigned int code, Param *param): code(code) {}

};