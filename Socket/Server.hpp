#pragma once

#include <sys/epoll.h>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>
#include <queue>
#include <sys/epoll.h>

#include "User.hpp"
#include "Channel.hpp"

#define READ_SIZE 420
#define MAX_EVENTS 69

extern int signal_server_stop;

class Server
{
private:
	bool				stop_server;
	int					sockfd;
	int					epollfd;
	struct epoll_event event, events[MAX_EVENTS];

	// Pool of users loaded from disk, used for authentication
	//std::vector<User>	loaded_users;

	size_t 				max_client_id;
	size_t				max_channel_id;

	std::vector<int>	client_fds;
	std::vector<User>	clients;
	std::vector<std::queue<std::tuple<void *, size_t, bool>>> messages;
	std::vector<Channel> servers;

	ssize_t get_user_index_by_fd(int fd);
	User *get_user_by_fd(int fd);

	void handle_read_event(int fd);
	void handle_write_event(int fd);
	void handle_event(const epoll_event event, int sockfd);

	void route_message(std::string msg, User &sender, size_t user_index);

public:
	Server();
	~Server();

	size_t	n_users();
	void disconnect_user(size_t user_index);
	void add_msg(void *msg, size_t len, bool is_heap, User &receiver);
	void add_msg(void *msg, size_t len, bool is_heap, Channel receivers);


	User &get_user_by_nick(std::string nick);
	User &get_user_by_id(size_t id);
	std::vector<User&> get_channel_users(const Channel channel);
	Channel &get_by_channel_name(std::string name);
	Channel &get_by_channel_id(size_t id);

	void stop();

	std::vector<User>	&getUsers(void);

	// Returns true on errors
	int loop(size_t PORT);
};
