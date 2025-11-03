#include <cstddef>
#include <string>
#include <vector>
#include <tuple>

#include "User.hpp"
#include "Channel.hpp"

class Server
{
private:
	bool				stop_server = 0;
	int					sock_fd = 0;

	// Pool of users loaded from disk, used for authentication
	std::vector<User>	loaded_users;

	size_t 				max_client_id = 0;
	size_t				max_server_id = 0;

	std::vector<int>	client_fds;
	std::vector<User>	clients;
	std::vector<Channel> servers;

	void write_user(User user, std::ofstream stream);
	User read_user();

	void route_message(std::string msg, User sender);

public:
	void set_stop(bool value);

	// 	Returns true on errors
	int write_data_to_file(std::string path);

	// Returns true on errors
	int load_from_file(std::string path);

	void handle_message(size_t client_i, void *msg, size_t msg_len);

	// Returns true on errors
	int loop(size_t PORT);
};
