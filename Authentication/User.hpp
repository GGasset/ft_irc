#pragma once

#include <cstddef>
#include <string>

#include "Server.hpp"

class User
{
private:
	// id set to -1 means user not registered
	ssize_t id = -1;
	bool is_channel_operator = false;
	std::string nick = "";
	std::string realname;
	std::string username;
	std::vector<size_t> joined_channels_ids;

	// current state of the received msg, may not be complete
	// Is not saved to disk
	std::string current_message;
	std::string passw;

public:
	// Called when this user sends a message
	// This function is part of the socket function collection
	std::vector<std::string> msg_sent(void *what_he_wants, size_t msg_len);
	std::string getNick(void) const;
	std::vector<size_t>	get_joined_channel(size_t id);
	void	setNick(std::string nick);
};
