#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "Server.hpp"

#define MAX_EVENTS 5

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

int Server::loop(size_t PORT)
{
	sockfd = setup_sockfd(PORT);
	if (sockfd == -1) return true;
	int epollfd = epoll_create1(0);

	struct epoll_event event, events[MAX_EVENTS];

	event.events = EPOLLIN;
	event.data.fd = sockfd;

	int err = 0;
	// Add sockfd for read watchlist to accept clients
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event)) err = true;

	while (!stop_server || err)
	{
		size_t event_n = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (event_n == -1) {err = true; continue;}

		for (size_t i = 0; i < event_n; i++)
			if (events[i].data.fd == sockfd) // Socket fd is redeable (someone is trying to connect)
			{
				int new_client_fd = accept(sockfd, 0, 0);

				// Add client
			}
			else
			{
				if (events[i].events & EPOLLIN)
					handle_read_event(events[i].data.fd);
				if (events[i].events & EPOLLOUT)
					handle_write_event(events[i].data.fd);
			}		
	}
	
	close(sockfd);
	close(epollfd);
	for (size_t i = 0; i < client_fds.size(); i++)
		close(client_fds[i]);
	return err;
}
