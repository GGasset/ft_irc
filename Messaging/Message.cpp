/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvaro <alvaro@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 19:24:29 by alvmoral          #+#    #+#             */
/*   Updated: 2025/11/24 13:25:36 by alvaro           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

msgs    getMsgs(std::string packet)
{
	size_t	pos;
	msgs	ret;
	std::string	new_line;

	while (1)
	{
		pos = packet.find("\r\n");
		if (pos == std::string::npos)
			break ;
		new_line = packet.substr(0, pos + 2);
		ret.push_back(new_line);
		packet = packet.substr(pos + 2);
	}
	if (!packet.empty())
	ret.push_back(packet + "\r\n");
	return (ret);
}

std::string	getWORD(std::string &packet, size_t &beginWord)
{
	std::string	ret;
	size_t	pos = packet.find(" ", beginWord);
	if (pos == std::string::npos)
		pos = packet.find("\r\n");
	ret = packet.substr(beginWord, pos - beginWord);
	beginWord = pos;
	return (ret);
}

std::string	getTRAIL(std::string &packet, size_t &beginWord)
{
	std::string	ret;
	size_t	pos = packet.find("\r\n", beginWord);
	ret = packet.substr(beginWord, pos - beginWord);
	beginWord = pos;
	return (ret);
}

std::string	getSPACE(std::string &packet, size_t &beginSpace)
{
	std::string::iterator it = packet.begin() + beginSpace;
	size_t tmp = beginSpace;
	for (; it != packet.end() && *it == ' '; it++)
		beginSpace++;
	
	std::string ret = tmp == beginSpace ? "" : " ";
	return (ret);
}

void	newSPACE(msgTokens &ret, std::string &msg, size_t &begin)
{
	std::string isSpace = getSPACE(msg, begin);
	if (isSpace.empty())
		return ;
	ret.push_back((msg_token) {SPACE , " "});
}

char iterStr(const std::string& str, bool restart) {
    static size_t index = 0;  // Guarda la posición entre llamadas sucesivas
    static const std::string* current = nullptr;

	if (restart)
		return index = 0, '\0';
    // Si cambia el string de entrada, reiniciamos el índice
    if (&str != current) {
        current = &str;
        index = 0;
    }

    // Si hemos llegado al final, reiniciamos y devolvemos '\0'
    if (index >= str.size()) {
        index = 0;
        current = nullptr;
        return '\0';
    }
    return str[index++];
}

bool	isNUMBER(const std::string &param) {
	char c;
	
	iterStr(param, 1);
	while (1) {
		c = iterStr(param, 0);
		if (!c)
			break ;
		if (c < '0' || c > '9')
			return (false);
	}
	return (true);
}

std::string	capitalize(const std::string &input) {
    std::string result = input;
    for (char& c : result) {
        c = static_cast<unsigned char>(std::toupper(c));
    }
    return result;
}

msgTokens	msgTokenizer(std::string msg)
{
	msgState	state = PRIX;
	size_t		begin = 0;
	msgTokens	ret;
	std::string param;

	while (1)
	{
		if (state == PAR)
		{
			if (msg[begin] == '\r'
				&& msg[begin + 1] == '\n')
			{
				ret.push_back((msg_token) {CRLF, "\r\n"});
				break ;
			}
			newSPACE(ret, msg, begin);
			if (msg[begin] == ':')
			{
				ret.push_back((msg_token) {TRAIL, getTRAIL(msg, begin).erase(0, 1)});
				continue ;
			}
			param = getWORD(msg, begin);
			if (param.find(",") != std::string::npos)
				ret.push_back((msg_token) {COMMA_LIST, param});
			else
				ret.push_back((msg_token) {TOK_PARAM, param});
		}
		else if (state == PRIX)
		{
			if (msg[0] == ':')
			{
				ret.push_back((msg_token) {PREFIX, getWORD(msg, begin)});
				newSPACE(ret, msg, begin);
			}
			state = CMD;
		}
		else if (state == CMD)
		{
			if (msg[begin] == '\r'
				&& msg[begin + 1] == '\n')
			{
				ret.push_back((msg_token) {CRLF, "\r\n"});
				break ;
			}
			else if (isspace(msg[begin]))
				newSPACE(ret, msg, begin);
			else
			{
				param = getWORD(msg, begin);
				ret.push_back((msg_token) {isNUMBER(param) ? NUMBER: WORD, capitalize(param)});
			}
			state = PAR;
		}
	}
	return (ret);
}

