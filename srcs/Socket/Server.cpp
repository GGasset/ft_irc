#include "Server.hpp"
#include <cassert>
#include <unistd.h>

void Server::stop()
{
	stop_server = true;
}

Server::Server()
{
	stop_server = false;
	sockfd = -1;
	epollfd = -1;
	max_client_id = 0;
	max_channel_id = 0;
}

Server::~Server()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		event.events = EPOLLIN | EPOLLOUT;
		event.data.fd = client_fds[i];
		epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fds[i], &event);

		close(client_fds[i]);
		while (messages[i].size())
		{
			if (std::get<2>(messages[i].front())) delete[] std::get<0>(messages[i].front());
			messages[i].pop();
		}
	}
	client_fds.clear();
	last_pong_time.clear();
	clients.clear();
	messages.clear();
	servers.clear();

	event.events = EPOLLIN;
	event.data.fd = sockfd;
	epoll_ctl(sockfd, EPOLL_CTL_DEL, sockfd, &event);

	close(sockfd);
	sockfd = -1;
	close(epollfd);
	epollfd = -1;
}

void Server::disconnect_user(size_t user_index)
{
	if (user_index >= clients.size()) throw;

	std::queue<std::tuple<void *, size_t, bool>> user_messages = messages[user_index];
	while (user_messages.size())
	{
		if (std::get<2>(user_messages.front()))
			delete[] std::get<0>(user_messages.front());
		user_messages.pop();
	}

	event.data.fd = client_fds[user_index];
	event.events = EPOLLIN | EPOLLOUT;
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fds[user_index], &event)) stop();

	close(client_fds[user_index]);

#ifndef DONT_LOG
	std::cout << clients[user_index].getUsername() << " disconnected" << std::endl;;
#endif

	client_fds.erase(client_fds.begin() + user_index);
	last_pong_time.erase(last_pong_time.begin() + user_index);
	messages.erase(messages.begin() + user_index);
	clients.erase(clients.begin() + user_index);

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

void Server::set_pong_time(size_t user_id)
{
	ssize_t index = get_user_index_by_id(user_id);
	if (index != -1) return;
	last_pong_time[index] = time(NULL);
}

ssize_t Server::get_user_index_by_id(size_t id)
{
	for (size_t i = 0; i < clients.size(); i++)
		if (clients[i].get_id() == id)
			return i;
	return -1;
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

void Server::send_pings()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (time(NULL) - last_pong_time[i] > USER_TIMEOUT_S)
		{
#ifndef DONT_LOG
			std::cout << "User timed out: ";
#endif
			disconnect_user(i);
			i--;
			continue;
		}
		if (last_pong_time[i] < last_ping_time) continue; // Don't queue pings, wait until client responded

		// send ping
		//add_msg(,,,clients[i]);
	}
	last_ping_time = std::time(NULL);
}

size_t	Server::n_users() {
	return clients.size();
}

void	Server::addUser(User u) {
	clients.push_back(u);
	client_fds.push_back(-1);
	messages.push_back(std::queue<std::tuple<void *, size_t, bool>>());
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

std::vector<User>	&Server::getUsers(void) {
	return (clients);
}
