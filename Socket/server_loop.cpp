#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>

#include "Server.hpp"


void handle_signals(int signal)
{
	signal_server_stop = true;
}

static int setup_sockfd(size_t PORT)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket fd

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (
	 	sockfd == -1
	 || bind(sockfd, (const sockaddr*)&addr, sizeof(addr))// Attach fd to PORT
	 || listen(sockfd, 20) // Mark fd as the one used to accept connections
	 || fcntl(sockfd, F_SETFL/*Set flags*/, fcntl(sockfd, F_GETFL/*Get flags*/, 0) | O_NONBLOCK) == -1 // Set non block
	) 
	{
		close(sockfd);
		return -1;
	}
	return sockfd;
}

void Server::handle_read_event(int fd)
{
	std::string read_data;
	char tmp[READ_SIZE + 1] {};

	while (read(fd, &tmp, READ_SIZE)) read_data += tmp;
	if (!read_data.length()) return;
	ssize_t sender_index = get_user_index_by_fd(fd);
	User *sender = get_user_by_fd(fd);
	if (!sender) return;
	std::vector<std::string> msgs = sender->msg_sent(read_data);
	for (size_t i = 0; i < msgs.size(); i++) route_message(msgs[i], *sender, sender_index);
}

void Server::handle_write_event(int fd)
{
	ssize_t user_i = get_user_index_by_fd(fd);
	if (user_i == -1) return;
	if (!messages[user_i].size()) return;

	std::tuple<void*,size_t,bool> next_msg = messages[user_i].front();
	messages[user_i].pop();

	write(fd, std::get<0>(next_msg), std::get<1>(next_msg));
	if (std::get<2>(next_msg)) delete[] std::get<0>(next_msg);
}

void Server::handle_event(const epoll_event event, int sockfd)
{
	if (event.data.fd == sockfd) // Socket fd is redeable (someone is trying to connect)
	{
		int new_client_fd = accept(sockfd, 0, 0);
		if (new_client_fd == -1
			|| fcntl(sockfd, F_SETFL/*Set flags*/, fcntl(sockfd, F_GETFL/*Get flags*/, 0) | O_NONBLOCK) == -1 // Set non block
		) return;

		// Add client
		client_fds.push_back(new_client_fd);
		clients.push_back(User());
		messages.push_back(std::queue<std::tuple<void *, size_t, bool>>());

		this->event.events = EPOLLIN | EPOLLOUT;
		this->event.data.fd = new_client_fd;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_client_fd, &this->event)) stop();
	}
	else
	{
		if (event.events & EPOLLIN)
			handle_read_event(event.data.fd);
		if (event.events & EPOLLOUT)
			handle_write_event(event.data.fd);
	}
}

int Server::loop(size_t PORT)
{
	signal_server_stop = false;
	signal(SIGTSTP, handle_signals);
	signal(SIGSTOP, handle_signals);
	signal(SIGINT, handle_signals);
	signal(SIGQUIT, handle_signals);
	signal(SIGTERM, handle_signals);

	sockfd = setup_sockfd(PORT);
	if (sockfd == -1) return true;
	epollfd = epoll_create1(0);

	event.events = EPOLLIN;
	event.data.fd = sockfd;

	int err = 0;
	// Add sockfd for read watchlist to accept clients
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event)) err = true;

	while (!stop_server && !err && !signal_server_stop)
	{
		size_t event_n = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (event_n == -1) {err = errno != EINTR; continue;}

		for (size_t i = 0; i < event_n; i++)
			handle_event(events[i], sockfd);
	}
	
	close(sockfd);
	close(epollfd);
	for (size_t i = 0; i < client_fds.size(); i++)
		close(client_fds[i]);
	return err;
}
