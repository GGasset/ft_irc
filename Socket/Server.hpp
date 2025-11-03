#include <cstddef>
#include <vector>
#include <tuple>

#include "User.hpp"

class Server
{
private:
	size_t 				max_id;

	std::vector<int>	client_fds;
	std::vector<User>	clients;

public:
};
