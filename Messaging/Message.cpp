/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvmoral <alvmoral@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 19:24:29 by alvmoral          #+#    #+#             */
/*   Updated: 2025/11/03 19:24:30 by alvmoral         ###   ########.fr       */
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

void comparar_vectores(const std::vector<std::string>& a,
                       const std::vector<std::string>& b)
{
    size_t max_size = std::max(a.size(), b.size());
    bool hay_diferencias = false;

    for (size_t i = 0; i < max_size; ++i) {
        std::string s1 = (i < a.size()) ? a[i] : "(<sin valor>)";
        std::string s2 = (i < b.size()) ? b[i] : "(<sin valor>)";

        if (s1 != s2) {
            hay_diferencias = true;
            std::cout << "❌ Diferencia en índice " << i << ":\n";
            std::cout << "   A: \"" << s1 << "\"\n";
            std::cout << "   TEST: \"" << s2 << "\"\n";
        }
    }

    if (!hay_diferencias)
        std::cout << "✅ No hay diferencias.\n";
}

bool compare_tokens(const msg_token& a, const msg_token& b) {
    bool iguales = true;

    if (a.type != b.type) {
        std::cout << "❌ Diferente tipo:\n";
        std::cout << "   A: " << a.type << " | B: " << b.type << '\n';
        iguales = false;
    }

    if (a.str != b.str) {
        std::cout << "❌ Diferente valor:\n";
        std::cout << "   A: \"" << a.str << "\" | B: \"" << b.str << "\"\n";
        iguales = false;
    }

    if (iguales)
        std::cout << "✅ Tokens iguales: " << a.str << '\n';

    return iguales;
}

bool compare_token_vectors(const std::vector<msg_token>& A,
                           const std::vector<msg_token>& B)
{
    bool iguales = true;
    size_t max_size = std::max(A.size(), B.size());

    for (size_t i = 0; i < max_size; ++i) {
        if (i >= A.size() || i >= B.size()) {
            std::cout << "❌ Diferente longitud de listas.\n";
            iguales = false;
            break;
        }
        if (!compare_tokens(A[i], B[i]))
            iguales = false;
    }

    return iguales;
}


void	print_msg_test(size_t test_id, msgs packet_msgs, msgs test)
{
	std::cout << "[" << test_id << "]: ";
	comparar_vectores(packet_msgs, test);
	std::cout << "------------------------------------\n";
}

void	print_part_test(size_t test_id, msgTokens mine, msgTokens test)
{
	std::cout << "[" << test_id << "]: " << std::endl;
	compare_token_vectors(mine, test);
	std::cout << "---------------------------------\n";
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

char iterStr(const std::string& str) {
    static size_t index = 0;  // Guarda la posición entre llamadas sucesivas
    static const std::string* current = nullptr;

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
	while ((c = iterStr(param))) {
		if (!isdigit(c))
			return (false);
	}
	return (true);
}

msgTokens	msgTokenizer(std::string msg)
{
	msgState	state = PRIX;
	size_t		begin = 0;
	msgTokens	ret;

	while (1)
	{
		if (state == PARAM)
		{
			if (msg[begin] == '\r'
				&& msg[begin + 1] == '\n')
				break ;
			newSPACE(ret, msg, begin);
			if (msg[begin] == ':')
			{
				ret.push_back((msg_token) {TRAIL, getTRAIL(msg, begin)});
				continue ;
			}
			std::string param = getWORD(msg, begin);
			if (param.find(",") != std::string::npos)
				ret.push_back((msg_token) {COMMA_LIST, param});
			else
				ret.push_back((msg_token) {isNUMBER(param) ? NUMBER: WORD, param});
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
			ret.push_back((msg_token) {WORD, getWORD(msg, begin)});
			state = PARAM;
		}
	}
	return (ret);
}

int main(void)
{
	std::string packet;
	msgs		packet_msgs;
	msgs		test;
    /* Test 1 */
    packet = "Rataaaaaaa  aa :asdsdff assd \r\nooianffo assd ";
    packet_msgs = getMsgs(packet);
    test = {"Rataaaaaaa  aa :asdsdff assd \r\n", "ooianffo assd \r\n"};
	// print_msg_test(1, packet_msgs, test);

	
	/* Test 2 */
    packet = "AAAAAAAAA\r\nBBBBBBB \r\n\nCC";
    packet_msgs = getMsgs(packet);
    test = {"AAAAAAAAA\r\n", "BBBBBBB \r\n", "\nCC\r\n"};
	// print_msg_test(2, packet_msgs, test);

	/* Test 3 */
    // packet = "";
    // packet_msgs = getMsgs(packet);
    // test = {"AAAAAAAAA", "BBBBBBB ", "\nCC"};
	// print_msg_test(3, packet_msgs, test);

	/* Test 4 */
    packet = "JOIN #cthullu\r\nTOPIC #cthullu :literatura de terror\r\n";
    packet_msgs = getMsgs(packet);
    test = {"JOIN #cthullu\r\n", "TOPIC #cthullu :literatura de terror\r\n"};
	// print_msg_test(4, packet_msgs, test);

	msgTokens testT = {
		(msg_token) {WORD, "JOIN"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "#cthullu"},
	};

	// print_part_test(4, msgTokenizer(packet_msgs[0]), testT);

	/* Test 5 */
	msgTokens testT2 = {
		(msg_token) {WORD, "TOPIC"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "#cthullu"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":literatura de terror"}
	};
	print_part_test(5, msgTokenizer(packet_msgs[1]), testT2);

	/* Test 6 */
	packet = "PING :irc.local\r\n";
	testT = {
		(msg_token) {WORD, "PING"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":irc.local"},
	};
	print_part_test(6, msgTokenizer(packet), testT);

	/* Test 7 */
	packet = ":NICK!user@host PRIVMSG #canal :Hola mundo\r\n";
	testT = {
		(msg_token) {PREFIX, ":NICK!user@host"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":Hola mundo"}
	};
	print_part_test(7, msgTokenizer(packet), testT);

	/* Test 8 */
	packet = "USER alvaro 0 * :Álvaro Martínez\r\n";
	testT = {
		(msg_token) {WORD, "USER"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "alvaro"},
		(msg_token) {SPACE, " "},
		(msg_token) {NUMBER, "0"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "*"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":Álvaro Martínez"}
	};
	print_part_test(8, msgTokenizer(packet), testT);

	/* Test 9 */
	packet = "JOIN #a,#b,#c keyA,keyB,keyC\r\n";
	testT = {
		(msg_token) {WORD, "JOIN"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "#a,#b,#c"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "keyA,keyB,keyC"},
	};
	print_part_test(9, msgTokenizer(packet), testT);

	/* Test 10 */
	packet = "MODE #a,+i,#b,-t\r\n";
	testT = {
		(msg_token) {WORD, "MODE"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "#a,+i,#b,-t"},
	};
	print_part_test(10, msgTokenizer(packet), testT);

	/* Test 11 */
	
}