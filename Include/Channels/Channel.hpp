#pragma once

#include <vector>
#include "aio.h"
#include <cstring>
#include "User.hpp"
#include "string"

struct channel_mode
{
	std::vector<size_t>	operator_user_id;
	std::string			topic;
	std::string			passw;
	ssize_t				user_limit;
	bool				invite_only;
	bool				topic_protected;
};

class Server;

class Channel
{
private:
	std::vector<size_t> 		member_user_ids;
	std::vector<User *> 		users;
	std::vector<size_t> 		invited_ids;
	std::vector<std::string>	history;
	std::string					key;
	std::string 				name;
	std::string					topic;
	channel_mode 				mode;
	bool						mode_active;
	ssize_t 					id;

public:
	Channel(void);
	Channel(std::string _name, User* _user, std::string _key);
	~Channel(void);
	ssize_t 					get_id(void);
	bool						is_operator(const User* user) const;
	bool						get_mode(void);
	bool						is_invited(const User* user) const;
	void						set_id(ssize_t id);
	void						set_key(char key);
	void						set_mode(std::string key);
	void						invite_user(User* user);
	void						set_topic(std::string topic, User *user, Server &server);
	void						kick_user(User *user);
	void						broadcast(Server& serv, const std::string &mgs);
	int							add_member(User* user, Server *s, std::string msg);
	std::string					get_topic(void);
	std::string					get_key(void);
	std::string 				get_name(void);
	const std::vector<User *>	&get_members(void) const;
	const std::vector<size_t>	&get_member_ids(void) const;
	const std::vector<size_t>	&get_invited_ids(void) const;
};
