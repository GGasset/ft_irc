#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Server.hpp"


int Server::loop(size_t PORT)
{
	sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sock_fd == -1) return true;

	sockaddr_in6 addr;
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons()
}
