#pragma once

#include <vector>
#include <cstring>
#include "User.hpp"
#include "string"

struct channel_mode
{
	// Empty passw for no passw
	std::string passw;
	// 0 for no limit
	size_t user_limit;
	std::vector<size_t> operator_user_id;
	std::string topic;
};

class Server;

class Channel
{
private:
	size_t id = -1;
	std::vector<size_t> member_user_ids;
	// I don't know if this is correct (would be used for users who just joined the channel and the msg history would be sent)
	// Just the msgs meant to be read by other people
	std::vector<std::string> history;
	std::vector<User *> users;
	channel_mode mode;
	std::string name;
	std::string	topic = "";

public:
	Channel(void);
	Channel(std::string _name);
	~Channel(void);
	ssize_t get_id(void);
	void set_id(ssize_t id);
	std::string get_topic(void);
	void set_topic(std::string topic);
	std::vector<size_t>	get_members(void);
	std::string get_name(void);

	void	broadcast(Server& serv, const std::string &mgs);
};
