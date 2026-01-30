
#include <cctype>
#include <cstdlib>
#include <exception>

#include "Server.hpp"
//#include "fnHandlers.hpp"
#include "Channel.hpp"
#include "User.hpp"

#define USAGE_STR "Usage: ./ircserv PORT PASSW"

int main(int argc, char **argv)
{
	if (argc != 3)
		{std::cerr << "Invalid parameters count. " << USAGE_STR << std::endl; return 0;}

	bool port_is_digits = !argv[1][0];
	for (size_t i = 0; argv[1][i]; i++)
		port_is_digits = port_is_digits && argv[1][i] >= '0' && argv[1][i] <= '9';
	if (port_is_digits)
		{std::cerr << "Invalid PORT paramter, only digits expected >:(" << std::endl; return 0;}
	int port = std::atoi(argv[1]);

	bool pass_is_alnum = argv[2][0];
	for (size_t i = 0; argv[2][i]; i++) pass_is_alnum = pass_is_alnum && std::isalnum(argv[2][i]);
	if (!pass_is_alnum)
		{std::cerr << "Invalid PASSW paramter, only alpha numeric digits expected >:(" << std::endl; return 0;}

	Server server(argv[2]);
	if (server.loop(port))
		{std::cerr << "Internal server err" << std::endl; return 0;}
}
