#pragma once

#include <vector>
#include "aio.h"
#include <cstring>
#include "User.hpp"
#include "string"

struct channel_mode
{
	// Empty passw for no passw
	std::string passw;
	// 0 for no limit
	ssize_t user_limit;
	std::vector<size_t> operator_user_id;
	std::string topic;
};

class Server;

class Channel
{
private:
	ssize_t id;
	std::vector<size_t> member_user_ids;
	// I don't know if this is correct (would be used for users who just joined the channel and the msg history would be sent)
	// Just the msgs meant to be read by other people
	std::vector<std::string> history;
	std::vector<User *> users;
	channel_mode mode;
	std::string name;
	std::string	topic;

public:
	Channel(void);
	Channel(std::string _name, User* _user);
	~Channel(void);
	ssize_t get_id(void);
	bool	is_operator(const User* user) const;
	void set_id(ssize_t id);
	std::string get_topic(void);
	void set_topic(std::string topic);
	const std::vector<User *>	&get_members(void) const;
	int	add_member(User* user, Server *s, std::string msg);
	std::string get_name(void);

	void	broadcast(Server& serv, const std::string &mgs);
};
