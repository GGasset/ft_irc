#pragma once

#include <cstddef>
#include <string>

#include "Server.hpp"

class User
{
private:
	// id set to -1 means user not registered
	ssize_t id = -1;
	std::string nick = "";
	std::vector<size_t> joined_channels_ids;

	// current state of the received msg, may not be complete
	// Is not saved to disk
	std::string current_message;

public:
	// Called when this user sends a message
	// This function is part of the socket function collection
	std::vector<std::string> msg_sent(std::string data);
	ssize_t get_id();
	void set_id(ssize_t id);
	std::string get_nick();
};
