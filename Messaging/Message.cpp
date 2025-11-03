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
	std::cout << "[" << test_id << "]: ";
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
	std::cout << "ret " << ret << "---";
	beginWord = pos;
	return (ret);
}

std::string	getTRAIL(std::string &packet, size_t &beginWord)
{
	std::string	ret;
	size_t	pos = packet.find("\r\n", beginWord);
	ret = packet.substr(beginWord, pos - beginWord);
	std::cout << "ret " << ret << "---";
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
				ret.push_back((msg_token) {TRAIL, getTRAIL(msg, begin)});
			else
				ret.push_back((msg_token) {WORD, getWORD(msg, begin)});
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
	print_msg_test(1, packet_msgs, test);

	
	/* Test 2 */
    packet = "AAAAAAAAA\r\nBBBBBBB \r\n\nCC";
    packet_msgs = getMsgs(packet);
    test = {"AAAAAAAAA\r\n", "BBBBBBB \r\n", "\nCC\r\n"};
	print_msg_test(2, packet_msgs, test);

	/* Test 3 */
    // packet = "";
    // packet_msgs = getMsgs(packet);
    // test = {"AAAAAAAAA", "BBBBBBB ", "\nCC"};
	// print_msg_test(3, packet_msgs, test);

	/* Test 4 */
    packet = "JOIN #cthullu\r\nTOPIC #cthullu :literatura\r\n";
    packet_msgs = getMsgs(packet);
    test = {"JOIN #cthullu\r\n", "TOPIC #cthullu :literatura\r\n"};
	print_msg_test(4, packet_msgs, test);

	msgTokens testT = {
		(msg_token) {WORD, "JOIN"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "#cthullu"},
	};

	print_part_test(4, msgTokenizer(packet_msgs[0]), testT);

	/* Test 5 */
    packet = "JOIN #cthullu\r\nTOPIC #cthullu :literatura de terror\r\n";
    packet_msgs = getMsgs(packet);
    test = {"JOIN #cthullu\r\n", "TOPIC #cthullu :literatura\r\n"};
	print_msg_test(4, packet_msgs, test);

	testT = {
		(msg_token) {WORD, "JOIN"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "#cthullu"},
	};

	print_part_test(4, msgTokenizer(packet_msgs[0]), testT);
}