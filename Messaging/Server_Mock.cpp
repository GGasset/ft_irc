#include "Server.hpp"
#include <cassert>
#include <unistd.h>

Server::Server() {
	
}

Server::~Server()
{
	client_fds.clear();
	clients.clear();
	servers.clear();
}

void Server::add_msg(void *msg, size_t len, bool is_heap, User &receiver)
{
	// assert(receiver.get_id() != -1);
	// ssize_t user_index = -1;
	// for (ssize_t i = 0; i < clients.size() && user_index != -1; i++)
	// 	user_index += (i - user_index) * (clients[i].get_id() == receiver.get_id());
	// if (user_index == -1) return;
	// messages[user_index].push(std::make_tuple(msg, len, is_heap));
}

std::vector<User>	&Server::getUsers(void) {
	return (clients);
}

User &Server::get_user_by_nick(std::string nick)
{
	for (size_t i = 0; i < clients.size(); i++) 
		if (clients[i].get_id() != -1 && clients[i].get_nick() == nick)
			return clients[i];	
	return (clients[0]);
}

User &Server::get_user_by_id(size_t id)
{
	return (clients[id]);
}

Channel &Server::get_by_channel_name(std::string name) {
	for (size_t i = 0; i < servers.size(); i++) 
		if (servers[i].get_id() != -1 && servers[i].get_name() == name)
			return servers[i];	
	return (servers[0]);
}

Channel &Server::get_by_channel_id(size_t id) {
	return (servers[id]);
}

void	Server::addUser(User u) {
	clients.push_back(u);
}

void	Server::addChannel(Channel ch) {
	servers.push_back(ch);
}

void	Server::addNickHistory(std::string nick) {
	nick_history.push_back(nick);
}

std::vector<std::string> Server::get_nick_history() {
	return nick_history;
}
