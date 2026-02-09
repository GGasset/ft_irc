
#ifndef USER_HPP
#define USER_HPP

#include <cstddef>
#include "aio.h"
#include <string>
#include <vector>
#include <iostream>

// #include "Server.hpp"

class User
{
private:
	ssize_t id;
	bool is_channel_operator;
	bool registered;
	bool passwd_match;
	std::string nick;
	std::string realname;
	std::string username;
	std::string hostname; //This came from getnameinfo or is the ip from the client; used by broadcastin commmands like in NICK, RPL_WELCOME, etc.
	std::vector<size_t> joined_channels_ids;

	// current state of the received msg, may not be complete
	std::string current_message;

public:
	User();
	User(std::string nick, size_t id);

	std::string get_nick() const;
	std::string getUsername(void) const;
	std::string getRealname(void) const;
	std::string getHostname(void) const;
	size_t	get_joined_channel(size_t id);
	std::vector<size_t>	get_joined_channels(void);

	void add_to_channel(size_t id);

	void	setNick(std::string nick);
	void	set_username(std::string username);
	void	set_realname(std::string realname);
	void	set_hostname(std::string hostname);
	bool	passwd_match_pop(bool cond);

	// Called when this user sends a message
	// This function is part of the socket function collection
	std::vector<std::string> msg_sent(std::string data);
	ssize_t 	get_id();
	void 		set_id(ssize_t id);
	bool		is_registered();
	void		register_user();
	bool		are_names_registered();
};

#endif
