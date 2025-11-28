#pragma once

#include <sys/epoll.h>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>
#include <queue>
#include <sys/epoll.h>
#include <iostream>

#include "User.hpp"
#include "Channel.hpp"

#define READ_SIZE 1

extern int signal_server_stop;

class Server
{
private:
	std::vector<int>	client_fds;
	std::vector<User>	clients;
	std::vector<Channel> servers;

	std::vector<std::string> nick_history;

public:
	Server();
	~Server();

	void add_msg(void *msg, size_t len, bool is_heap, User &receiver);
	void add_msg(void *msg, size_t len, bool is_heap, Channel receivers);

	User &get_user_by_nick(std::string nick);
	User &get_user_by_id(size_t id);
	size_t	n_users();
	std::vector<User&> get_channel_users(const Channel channel);
	Channel &get_by_channel_name(std::string name);
	Channel &get_by_channel_id(size_t id);
	std::string passw;
	// std::string	get_server_password();

	void	addUser(User u);
	void	addChannel(Channel ch);
	void	addNickHistory(std::string nick);
	std::vector<std::string> get_nick_history();
	

	void stop();

	std::vector<User>	&getUsers(void);
};
