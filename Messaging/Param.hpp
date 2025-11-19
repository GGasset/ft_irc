#pragma once

#include "Message.hpp"

class Param {
	virtual ~Param() {};
	// virtual COMMAND command() const = 0
};

class NickParam: public Param {
	std::string nickname;

	public:
		NickParam(std::string nickname): nickname(nickname) {}
		~NickParam() {}
		NickParam& operator=(const NickParam& other) {
			if (this != &other)
				nickname = other.nickname;
			return (*this);
		}
};

class UserParam: public Param {
	std::string nickname;
	std::string realname;
	std::string username;
	std::string hostname;
	// std::string servername;

	public:
		UserParam(std::string nickname,
				  std::string username,
				  std::string realname,
				  std::string hostname): nickname(nickname),
										 username(username),
										 realname(realname),
										 hostname(hostname) {}

		~UserParam() {}
		UserParam& operator=(const UserParam& other) {
			if (this != &other)
				nickname = other.nickname;
				username = other.username;
				realname = other.realname;
				hostname = other.hostname;
			return (*this);
		}
};