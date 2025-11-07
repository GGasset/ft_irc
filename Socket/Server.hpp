#include <sys/epoll.h>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>
#include <queue>

#include "User.hpp"
#include "Channel.hpp"

#define READ_SIZE 1

class Server
{
private:
	bool				stop_server;
	int					sockfd;

	// Pool of users loaded from disk, used for authentication
	std::vector<User>	loaded_users;

	size_t 				max_client_id;
	size_t				max_channel_id;

	std::vector<int>	client_fds;
	std::vector<User>	clients;
	std::vector<std::queue<std::tuple<void *, size_t, bool>>> messages;
	std::vector<Channel> servers;

	ssize_t get_user_index_by_fd(int fd);
	User &get_user_by_fd(int fd);

	void handle_read_event(int fd);
	void handle_write_event(int fd);
	void handle_event(const epoll_event event, int sockfd);


	void write_user(User user, std::ofstream stream);
	User read_user();

	void route_message(std::string msg, User &sender);


public:
	void add_msg(void *msg, size_t len, bool is_heap, User &receiver);
	void add_msg(void *msg, size_t len, bool is_heap, Channel receivers);


	User &get_user_by_nick(std::string nick);
	std::vector<User&> get_channel_users(const Channel channel);
	Channel &get_by_channel_name(std::string name);

	void stop();

	// 	Returns true on errors
	int write_data_to_file(std::string path);

	// Returns true on errors
	int load_from_file(std::string path);

	void handle_message(size_t client_i, void *msg, size_t msg_len);

	// Returns true on errors
	int loop(size_t PORT);
};
