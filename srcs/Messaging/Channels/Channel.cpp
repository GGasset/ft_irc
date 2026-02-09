#include "Channel.hpp"
#include "Server.hpp"
#include "router.hpp"
#include <algorithm>
#include <string>

Channel::Channel(void){}
Channel::Channel(std::string _name, User* _user, std::string _key) : name(_name), id(-1)
{
	this->name = _name;
	this->key = _key;
	this->mode.passw = "";
	this->mode.topic = "";
	this->mode_active = false;
	this->mode.need_pass = false;
	this->mode.has_limit = false;
	this->mode.invite_only = false;
	this->mode.topic_protected = false;
	member_user_ids.push_back(_user->get_id());
	users.push_back(_user);
	mode.operator_user_id.clear();
	mode.operator_user_id.push_back(_user->get_id());
	this->topic = "";
}

Channel::~Channel(void){}
ssize_t Channel::get_id() {return id;}
void Channel::set_id(ssize_t id) {this->id = id;}
const std::vector<User *>	&Channel::get_members() const {return users;}
// const std::vector<size_t>& Channel::get_member_ids() const { return member_user_ids; }
std::string	Channel::get_key(void) {return key;}
void	Channel::set_key(char key){this->key = key;}
void	Channel::set_pass(std::string pass) {this->mode.passw = pass;}
void	Channel::set_operator(Server& server, const std::string& user)
{
    if (user.empty()) return;
    User *u = server.get_user_by_nick(user);
    if (!u)
		return;
    ssize_t uid_ss = u->get_id();
    if (uid_ss < 0) return;
    size_t uid = static_cast<size_t>(uid_ss);
    if (std::find(member_user_ids.begin(), member_user_ids.end(), uid) == member_user_ids.end())
        return;
    if (std::find(mode.operator_user_id.begin(), mode.operator_user_id.end(), uid) == mode.operator_user_id.end())
        mode.operator_user_id.push_back(uid);
}

void Channel::unset_operator(Server& server, const std::string& user)
{
    if (user.empty()) return;
    User *u = server.get_user_by_nick(user);
    if (!u) return;
    ssize_t uid_ss = u->get_id();
    if (uid_ss < 0) return;
    size_t uid = static_cast<size_t>(uid_ss);
    std::vector<size_t>::iterator it = std::find(mode.operator_user_id.begin(), mode.operator_user_id.end(), uid);
    if (it != mode.operator_user_id.end())
        mode.operator_user_id.erase(it);
}

bool	Channel::get_mode(void) {return this->mode_active;}
std::string Channel::get_name() {return name;}
std::string Channel::get_pass() {return mode.passw;}
const std::vector<size_t> &Channel::get_member_ids() const { return member_user_ids; }
std::string Channel::get_topic() {return topic;}
const std::vector<size_t> &Channel::get_invited_ids(void) const { return invited_ids; }

void	Channel::set_limit(ssize_t limit)
{
	this->mode.has_limit = true;
	this->mode.user_limit = limit;
}

void Channel::unset_limit()
{
	this->mode.has_limit = false;
}

void        Channel::set_topic(std::string topic, User *user, Server& server)
{
	if (!is_operator(user) && this->mode.topic_protected == true)
	{
		server.add_msg(std::string(RED"482 #canal :You're not channel operator (+t)" RESET), *user);
	}
	else
		this->topic = topic;
}
void Channel::set_mode(std::string key)
{
	this->mode_active = true;
	if (key == "+i")
		this->mode.invite_only = true;
	else if (key == "-i")
		this->mode.invite_only = false;
	else if (key == "+t")
		this->mode.topic_protected = true;
	else if (key == "-t")
		this->mode.topic_protected = false;
	else if (key == "+k")
		this->mode.need_pass = true;
	else if (key == "-k")
	{
		this->mode.need_pass = false;
		this->mode.passw = "";
	}
}

void Channel::invite_user(User* user)
{
    if (!user) return;
    size_t uid = static_cast<size_t>(user->get_id());
    for (size_t i = 0; i < invited_ids.size(); ++i)
        if (invited_ids[i] == uid)
            return;
    invited_ids.push_back(uid);
}

void Channel::kick_user(User* user)
{
	if (!user) return;
	size_t uid = static_cast<size_t>(user->get_id());
	std::vector<size_t>::iterator it_id = std::find(member_user_ids.begin(), member_user_ids.end(), uid);
	if (it_id != member_user_ids.end())
		member_user_ids.erase(it_id);
	std::vector<User*>::iterator it_user = std::find(users.begin(), users.end(), user);
	if (it_user != users.end())
		users.erase(it_user);
	std::vector<size_t>::iterator it_op = std::find(mode.operator_user_id.begin(), mode.operator_user_id.end(), uid);
	if (it_op != mode.operator_user_id.end())
		mode.operator_user_id.erase(it_op);
	std::vector<size_t>::iterator it_inv = std::find(invited_ids.begin(), invited_ids.end(), uid);
	if (it_inv != invited_ids.end())
		invited_ids.erase(it_inv);
}

bool Channel::is_invited(const User* user) const
{
    if (!user) return false;
    size_t uid = static_cast<size_t>(const_cast<User*>(user)->get_id());
    for (size_t i = 0; i < invited_ids.size(); ++i)
        if (invited_ids[i] == uid)
            return true;
    return false;
}

int Channel::add_member(User* user, Server *s, std::string msg, command_args args)
{
	if (!user) return 1;
	ssize_t uid = user->get_id();
    if (mode.need_pass)
    {
        if (args.argv.size() <= 2 || args.argv[2].empty())
        {
            if (s)
                s->add_msg(std::string(RED) + "482 " + get_name() + " :Password needed (+k)" + RESET, *user);
            return 1;
        }
        if (args.argv[2] != mode.passw)
        {
            if (s)
                s->add_msg(std::string(RED) + "475 " + get_name() + " :Bad channel key" + RESET, *user);
            return 1;
        }
    }
	if (mode.invite_only)
    {
        bool invited = is_invited(user);
        bool already_member = false;
        for (size_t i = 0; i < member_user_ids.size(); ++i)
            if (member_user_ids[i] == (size_t)uid)
            {
                already_member = true;
                break;
            }
        if (!invited && !already_member)
        {
            if (s)
                s->add_msg(std::string(RED"473 :Cannot join channel (+i)" RESET), *user);
            return 1;
        }
    }
	if (mode.has_limit && static_cast<ssize_t>(member_user_ids.size()) >= mode.user_limit)
    {
        if (s)
            s->add_msg(std::string(RED) + "471 " + get_name() + " :Channel is full (+l)" + RESET, *user);
        return 1;
    }

    for (size_t i = 0; i < member_user_ids.size(); ++i)
        if (member_user_ids[i] == (size_t)uid)
		{
			s->add_msg(msg, *user);
            return 1;
		}
    member_user_ids.push_back(uid);
    users.push_back(user);
    user->add_to_channel(id);
	if (s)
		s->add_msg(std::string(GREEN "Welcome to channel ") + get_name() + RESET, *user);
	return 0;
}

bool Channel::is_operator(const User* user) const
{
	if (!user) return false;
	size_t uid = static_cast<size_t>(const_cast<User*>(user)->get_id());
	for (size_t i = 0; i < mode.operator_user_id.size(); ++i)
		if (mode.operator_user_id[i] == uid)
			return true;
	return false;
}

/*
	boradcast (aun debo hacer el join bien)

	params:
			Server& serv -> referencia a server que controla los clientes y colas de salida,
			se llama para obtener el usuario destino y encolar el envio

			string &msg -> texto que se tiene que enviar a cada miembro del canal


	loop:
			obtener user id y guardar direccion en cather
			copiar bytes de msg a buffer
			encolar el mensaje y marcar true para asegurar que el serv libera el buffer

	return: void
	exception:
			si !catcher salta ese user
*/
void	Channel::broadcast(Server& serv, const std::string &msg)
{
	std::vector<size_t>	users;
	//size_t				len;
	User				*catcher;
	//char				*buffer;

	users = member_user_ids;
	for (size_t i = 0; i < users.size(); i++)
	{
		catcher = serv.get_user_by_id(users[i]);
		if (!catcher)
			continue;
		// len = msg.size();
		// buffer = new char[len];
		// std::memcpy(buffer, msg.data(), len);
		// serv.add_msg(buffer, len, true, *catcher);
		serv.add_msg(msg, *catcher);
	}
}

bool Channel::is_member(size_t user_id)
{
	for (size_t i = 0; i < member_user_ids.size(); i++)
		if (member_user_ids[i] == user_id) return true;
	return false;
}
