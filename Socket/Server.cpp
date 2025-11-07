#include "Server.hpp"
#include <cassert>

void Server::stop()
{
	stop_server = true;
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
