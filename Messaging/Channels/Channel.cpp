#include "Channel.hpp"

ssize_t Channel::get_id() {return id;}
void Channel::set_id(ssize_t id) {id = id;}
std::vector<size_t>	Channel::get_members() {return member_user_ids;}
std::string Channel::get_name() {return name;}