#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <cctype>
#include "cerrno"

#include "Server.hpp"

int signal_server_stop;


void handle_signals(int signal)
{
	signal = 0;
	(void) signal;
	signal_server_stop = true;
}

std::string sanitize(std::string in)
{
	std::string out;
	for (size_t i = 0; i < in.size(); i++)
	{
		if (std::isprint(in[i]))
		{
			if (std::isspace(in[i]))
				out += " ";
			else
				out += in[i];
		}
	}
	return out;
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
	 || fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1 // Set non block
	)
	{
		std::cerr << "bind err" << std::endl;
		close(sockfd);
		return -1;
	}
	return sockfd;
}

void Server::handle_read_event(int fd)
{
	std::string data;
	char tmp[READ_SIZE + 1];

	ssize_t bytes_read;
	do
	{
		for (size_t i = 0; i < READ_SIZE; i++) tmp[i] = 0;
		bytes_read = read(fd, tmp, READ_SIZE);
		data += tmp;
	}
	while (bytes_read == READ_SIZE);
	//std::cout << "Raw read: " << data << std::endl;

	{
		std::string str_tmp;
		for (size_t i = 0; replace_LF_to_CRLF && i < data.size(); i++)
		{
			if (data.size() == 1 && data[0] == '\n' && replace_LF_to_CRLF)
				str_tmp += "\r\n";
			else if (i && data[i - 1] != '\r' && data[i] == '\n' && replace_LF_to_CRLF)
				str_tmp += "\r\n";
			else
				str_tmp += data[i];
		}
		if (replace_LF_to_CRLF)
			data = str_tmp;
	}

	if (!data.length()) return;

	if (!fd)
	{
		std::string tmp_str;
		for (size_t i = 0; data[i]; i++)
		{
			tmp_str += data[i];
			if (data[i] == '\n' || i == data.size() - 1)
			{
				tmp_str = sanitize(tmp_str);
				if (tmp_str == "q" || tmp_str == "Q" || tmp_str == "quit" || tmp_str == "Quit")
					stop_server = true;

				if (tmp_str == "no ping" || tmp_str == "why") {send_pings_actively = false; std::cout << "Stopped active pinging" << std::endl;}
				if (tmp_str == "ping") {send_pings_actively = true; for (size_t i = 0; i < clients.size(); i++) set_pong_time(clients[i].get_id()); std::cout << "Started pinging actively" << std::endl;}
				if (tmp_str == "no crlf" || tmp_str == "why") {replace_LF_to_CRLF = true; std::cout << "Get those filthy CRLF away from me!" << std::endl;}
				if (tmp_str == "crlf") {replace_LF_to_CRLF = false; std::cout << "Whatever, activate CRLF requirements, by the way, will you also write quit? Please." << std::endl;}
				tmp_str = "";
			}
		}
		return;
	}

	ssize_t sender_index = get_user_index_by_fd(fd);
	User *sender = get_user_by_fd(fd);
	if (!sender) return;

	std::vector<std::string> msgs = sender->msg_sent(data);
	for (size_t i = 0; i < msgs.size(); i++) {
#ifndef DONT_LOG
		std::string msg = msgs[i];
		std::cout << std::endl << "Msg received from " << sender->getUsername() << ": " << sanitize(msg) << std::endl;
#endif
		route_message(msgs[i], *sender, sender_index);
	}
}

void Server::handle_write_event(int fd)
{
	ssize_t user_i = get_user_index_by_fd(fd);

	if (user_i == -1) return;
	if (!messages[user_i].size()) return;

	std::string next_msg = messages[user_i].front();
	messages[user_i].pop();

	#ifndef DONT_LOG
		std::cout << "Sending message to " << clients[user_i].get_nick() << ": " << sanitize(next_msg) << std::endl;
	#endif

	write(fd, next_msg.data(), next_msg.size() + 1);
}

void Server::handle_event(const epoll_event event, int sockfd)
{
	if (event.data.fd == sockfd) // Socket fd is redeable (someone is trying to connect)
	{
		int new_client_fd = accept(sockfd, 0, 0);
		if (new_client_fd == -1
		 || fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1 // Set non block
		) return;

#ifndef DONT_LOG
		std::cout << "Bluetooth device aconnected successfully" << std::endl;
#endif

		// Add client
		client_fds.push_back(new_client_fd);
		clients.push_back(User("", max_client_id));
		last_pong_time.push_back(std::time(NULL));
		max_client_id++;
		messages.push_back(std::queue<std::string>());

		this->event.events = EPOLLIN | EPOLLOUT;
		this->event.data.fd = new_client_fd;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_client_fd, &this->event)) stop();

		add_msg("NOTICE " + (std::string)"unregistered" + "\nWelcome! Go ahead and login with PASS [passw].\nYou may also write HELP to get a list of commands", clients.end()[-1]);
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
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) err = true;

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = 0;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, 0, &event)) err = true;

#ifndef DONT_LOG
	std::cout << "Bluetooth device is ready to peal at " << PORT << std::endl;
#endif

	last_ping_time = std::time(0);
	while (!stop_server && !err && !signal_server_stop)
	{
		int event_n = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
		if (event_n == -1) {err = errno != EINTR; continue;}

		if (send_pings_actively && std::time(0) - last_ping_time >= PING_SEPARATION_S)
			send_pings();

		for (int i = 0; i < event_n; i++)
			handle_event(events[i], sockfd);
	}

	close(sockfd);
	close(epollfd);
	for (size_t i = 0; i < client_fds.size(); i++)
		close(client_fds[i]);
	std::cout << "\nBye!" << std::endl;
	return err;
}
