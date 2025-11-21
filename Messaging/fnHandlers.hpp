#pragma once

#include "MessageOut.hpp"

class fnHandlers
{
	MessageOut (*fun[COMMAND0])(size_t, MessageIn, Server&);

	public:
		fnHandlers();
		~fnHandlers();
		MessageOut	operator()(COMMAND cmd, MessageIn msg, Server& server);
		
};

// extern const std::vector<MessageOut> g_Handler_Queue;

/* Manejadores */
// MessageOut handleNick(size_t clientId, MessageIn in, Server &server);