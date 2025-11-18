#include "User.hpp"

std::vector<std::string> User::msg_sent(std::string data)
{
	std::vector<std::string> out;
	for (size_t i = 0; i < data.length(); i++)
	{
		char prev = i? -1 : data[i - 1];
		if (prev == -1 && current_message.size()) prev = current_message.back(); 

		char c = data[i];

		current_message.push_back(c);
		if (prev < 0 || prev != '\r' || c != '\n')
			continue; // Message not completed

		// Empty message
		if (i == 2 && current_message.size() == 2) {current_message.clear(); continue;}

		out.push_back(current_message);
		current_message.clear();
	}
	return out;
}

std::vector<size_t>	User::get_joined_channel(size_t id) {
	return (joined_channels_ids);
}

std::string User::getNick(void) const{
	return (nick);
}

void	User::setNick(std::string nick) {
	this->nick = nick;
}

std::vector<size_t>	User::get_joined_channel(size_t id) {
	// return (joined_channels_ids[id]);
}