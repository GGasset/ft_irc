#pragma once

#include <cstddef>
#include <string>

#include "Server.hpp"

class User
{
private:
	// current state of the received msg, may not be complete
	std::string current_message;

	// Called when the user sent a complete message by msg_sent
	// Should parse the message and call the corresponding functions
	// When called, current message will contain a completed message, no need to empty it
	void msg_completed();
public:
	// Called when this user sent a message
	// This function is part of the socket function collection
	void msg_sent(void *what_he_wants, size_t msg_len);
};
