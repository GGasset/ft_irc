#include "Channel.hpp"

Channel::Channel(void){}
Channel::~Channel(void){}
ssize_t Channel::get_id() {return id;}
void Channel::set_id(ssize_t id) {id = id;}
std::vector<size_t>	Channel::get_members() {return member_user_ids;}
std::string Channel::get_name() {return name;}
std::string Channel::get_topic() {return topic;}
void        Channel::set_topic(std::string topic) {topic = topic;}
