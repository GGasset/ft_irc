#include "Server.hpp"
#include <cassert>
#include <unistd.h>

void Server::stop()
{
	stop_server = true;
}

Server::~Server()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		close(client_fds[i]);
		while (messages[i].size())
		{
			// if (std::get<2>(messages[i].front())) delete[] std::get<0>(messages[i].front());
			messages[i].pop();
		}
	}
	client_fds.clear();
	clients.clear();
	messages.clear();
	servers.clear();
}

void Server::disconnect_user(size_t user_index)
{
	if (user_index >= clients.size()) throw;
}

void Server::add_msg(void *msg, size_t len, bool is_heap, User &receiver)
{
	assert(receiver.get_id() != -1);
	ssize_t user_index = -1;
	for (ssize_t i = 0; i < clients.size() && user_index != -1; i++)
		user_index += (i - user_index) * (clients[i].get_id() == receiver.get_id());
	if (user_index == -1) return;
	messages[user_index].push(std::make_tuple(msg, len, is_heap));
}

User &Server::get_user_by_nick(std::string nick)
{
	for (size_t i = 0; i < clients.size(); i++) 
		if (clients[i].get_id() != -1 && clients[i].get_nick() == nick)
			return clients[i];	
	return (clients[0]);
}

ssize_t Server::get_user_index_by_fd(int fd)
{
	for (ssize_t i = 0; i < client_fds.size(); i++)
	{
		int iter_fd = client_fds[i];
		if (iter_fd == fd) return i;
	}
	return -1;
}

User *Server::get_user_by_fd(int fd)
{
	ssize_t user_index = get_user_index_by_fd(fd);
	if (user_index) return 0;
	return &clients[user_index];
}
