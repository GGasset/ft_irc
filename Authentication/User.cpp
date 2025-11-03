#include "User.hpp"

std::vector<std::string> User::msg_sent(void *what_he_wants, size_t msg_len)
{
	if (!what_he_wants || !msg_len) return;

	std::vector<std::string> out;
	for (size_t i = 0; i < msg_len; i++)
	{
		char prev = i? -1 : ((char*)what_he_wants)[i - 1];
		if (prev == -1 && current_message.size()) prev = current_message.back(); 

		char c = ((char*)what_he_wants)[i];

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