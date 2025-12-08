#pragma once

#include <sys/epoll.h>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>
#include <queue>
#include <sys/epoll.h>

#include <ctime>

#include "User.hpp"
#include "Channel.hpp"
#include "iostream"

#define READ_SIZE 420
#define MAX_EVENTS 69
#define USER_TIMEOUT_S 42
#define N_PINGS_UNTIL_TIMEOUT 5
#define PING_SEPARATION_S USER_TIMEOUT_S / N_PINGS_UNTIL_TIMEOUT - 1

extern int signal_server_stop;

class Server
{
private:
	size_t				last_ping_time;
	bool				stop_server;
	int					sockfd;
	int					epollfd;
	struct epoll_event event, events[MAX_EVENTS];

	// Pool of users loaded from disk, used for authentication
	//std::vector<User>	loaded_users;

	size_t 				max_client_id;
	size_t				max_channel_id;

	std::vector<int>	client_fds;
	std::vector<size_t>	last_pong_time; // TODO: set during message handling
	std::vector<User>	clients;
	std::vector<std::queue<std::tuple<void *, size_t, bool>>> messages;
	std::vector<Channel> servers;

	std::vector<std::string> nick_history;

	ssize_t get_user_index_by_fd(int fd);
	User *get_user_by_fd(int fd);

	void handle_read_event(int fd);
	void handle_write_event(int fd);
	void handle_event(const epoll_event event, int sockfd);

	void route_message(std::string msg, User &sender, size_t user_index);

	// Only sends to users who have responded previous pings, also disconnects users who timeout
	void send_pings(); // TODO: actually send the message

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

	std::string passw;
	// std::string	get_server_password();

	void	addUser(User u);
	void	addChannel(Channel ch);
	void	addNickHistory(std::string nick);
	std::vector<std::string> get_nick_history();

	void stop();

	std::vector<User>	&getUsers(void);

	// Returns true on errors
	int loop(size_t PORT);
};
