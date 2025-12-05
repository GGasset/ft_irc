#pragma once

#include "MessageOut.hpp"
#include "MessageIn.hpp"
#include <deque>

class fnHandlers
{
	MessageOut *(*fun[COMMAND0])(MessageIn, Server&);

	public:
		fnHandlers();
		~fnHandlers();
		MessageOut	*operator()(COMMAND cmd, MessageIn in, Server& server);
		
};

/* Manejadores */
MessageOut	*handleNick(MessageIn in, Server &server);

/* Utilidades de Manejadores */;
void    	complete_registry(User user, Server &server, UserParam *param);