
#include <cstdlib>


#include "Server.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include "User.hpp"

#define USAGE_STR "irc PORT [passw]"

int main(int argc, char **argv)
{
	if (2 > argc || argc > 3) 
		{std::cerr << "Invalid parameters " << USAGE_STR << std::endl; return 0;}

	bool port_is_digits = !argv[1][0];
	for (size_t i = 0; argv[1][i]; i++)
		port_is_digits = port_is_digits && argv[1][i] >= '0' && argv[1][i] <= '9';
	if (port_is_digits) 
		{std::cerr << "Invalid port paramter, only digits expected" << std::endl; return 0;}

	int port = std::atoi(argv[1]);

	Server server;
	if (server.loop(port))
		{std::cerr << "Internal server err" << std::endl; return 0;}
}
