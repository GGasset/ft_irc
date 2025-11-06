#include "Server.hpp"

std::vector<User>	&Server::getUsers(void) {
	return (clients);
}