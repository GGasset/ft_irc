#pragma once

#include <vector>
#include "string"

struct channel_mode
{
	// Empty passw for no passw
	std::string passw;
	// 0 for no limit
	size_t user_limit;
	std::vector<size_t> operator_user_id;
	std::string topic;
};

class Channel
{
private:
	std::vector<size_t> member_user_ids;

	// I don't know if this is correct (would be used for users who just joined the channel and the msg history would be sent)
	// Just the msgs meant to be read by other people
	std::vector<std::string> history;

	channel_mode mode;
	std::string name;

public:
};