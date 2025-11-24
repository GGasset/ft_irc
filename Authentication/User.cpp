#include "User.hpp"

User::User() {}

User::User(std::string nick, size_t id): nick(nick), id(id)  {}

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

size_t	User::get_joined_channel(size_t id) {
	return (joined_channels_ids[id]);
}

std::vector<size_t>	User::get_joined_channels(void) {
	return (joined_channels_ids);
}

ssize_t User::get_id() {
	return (id);
}

void User::set_id(ssize_t id) {
	this->id = id;
}

std::string User::get_nick(void) const{
	return (nick);
}

void	User::setNick(std::string nick) {
	this->nick = nick;
}


std::string User::getUsername(void) const {
	return (username);
}

std::string User::getRealname(void) const {
	return (realname);
}

bool	User::is_registered() {
    if (!get_nick().empty()
    && !getUsername().empty()
    && !getRealname().empty())
		return (true);
	return (false);
}