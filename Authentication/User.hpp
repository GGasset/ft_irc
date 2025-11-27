#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

// #include "Server.hpp"

class User
{
private:
	// id set to -1 means user not registered
	ssize_t id = -1;
	bool is_channel_operator = false;
	bool registered = false;
	std::string nick = "";
	std::string realname;
	std::string username;
	std::string hostname = ""; //This came from getnameinfo or is the ip from the client; used by broadcastin commmands like in NICK, RPL_WELCOME, etc.
	std::vector<size_t> joined_channels_ids;

	// current state of the received msg, may not be complete
	// Is not saved to disk
	std::string current_message;
	std::string passw;

public:
	User();
	User(std::string nick, size_t id);
	User &operator=(const User &other);

	std::string get_nick() const;
	std::string getUsername(void) const;
	std::string getRealname(void) const;
	std::string getHostname(void) const;
	size_t	get_joined_channel(size_t id);
	std::vector<size_t>	get_joined_channels(void);
	void	setNick(std::string nick);
	void	set_username(std::string username);
	void	set_realname(std::string realname);
	
	// Called when this user sends a message
	// This function is part of the socket function collection
	std::vector<std::string> msg_sent(std::string data);
	ssize_t 	get_id();
	void 		set_id(ssize_t id);
	bool		is_registered();
	void		register_user();
	bool		are_names_registered();
};
