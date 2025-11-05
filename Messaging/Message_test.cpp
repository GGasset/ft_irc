#include "Message.hpp"

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
}

int main(void)
{
	std::string packet;
	msgs		packet_msgs;
	msgs		test;
	msgTokens	testT;
	MessageIn	in;
	msgTokens	tokens;
	ParseStatus	status = VALID_MSG;

    /* Test 1 */
    // packet = "Rataaaaaaa  aa :asdsdff assd \r\nooianffo assd ";
    // packet_msgs = getMsgs(packet);
    // test = {"Rataaaaaaa  aa :asdsdff assd \r\n", "ooianffo assd \r\n"};
	// // print_msg_test(1, packet_msgs, test);

	
	// /* Test 2 */
    // packet = "AAAAAAAAA\r\nBBBBBBB \r\n\nCC";
    // packet_msgs = getMsgs(packet);
    // test = {"AAAAAAAAA\r\n", "BBBBBBB \r\n", "\nCC\r\n"};
	// print_msg_test(2, packet_msgs, test);

	/* Test 3 */
    // packet = "";
    // packet_msgs = getMsgs(packet);
    // test = {"AAAAAAAAA", "BBBBBBB ", "\nCC"};
	// print_msg_test(3, packet_msgs, test);

	/* Test 4 */
    // packet = "JOIN #cthullu\r\nTOPIC #cthullu :literatura de terror\r\n";
    // packet_msgs = getMsgs(packet);
    // test = {"JOIN #cthullu\r\n", "TOPIC #cthullu :literatura de terror\r\n"};
	// // print_msg_test(4, packet_msgs, test);

	// testT = {
	// 	(msg_token) {WORD, "JOIN"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {WORD, "#cthullu"},
	// };

	// // print_part_test(4, msgTokenizer(packet_msgs[0]), testT);

	// /* Test 5 */
	// msgTokens testT2 = {
	// 	(msg_token) {WORD, "TOPIC"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {WORD, "#cthullu"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {TRAIL, ":literatura de terror"}
	// };
	// print_part_test(5, msgTokenizer(packet_msgs[1]), testT2);

	// /* Test 6 */
	// packet = "PING :irc.local\r\n";
	// testT = {
	// 	(msg_token) {WORD, "PING"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {TRAIL, ":irc.local"},
	// };
	// print_part_test(6, msgTokenizer(packet), testT);

	// /* Test 7 */
	// packet = ":NICK!user@host PRIVMSG #canal :Hola mundo\r\n";
	// testT = {
	// 	(msg_token) {PREFIX, ":NICK!user@host"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {WORD, "PRIVMSG"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {WORD, "#canal"},
	// 	(msg_token) {SPACE, " "},
	// 	(msg_token) {TRAIL, ":Hola mundo"}
	// };
	// print_part_test(7, msgTokenizer(packet), testT);

	/* Test 8 */
	packet = "USER alvaro 0 * :Álvaro Martínez\r\n";
	testT = {
		(msg_token) {WORD, "USER"},
		(msg_token) {SPACE, " "},
		(msg_token) {PARAM, "alvaro"},
		(msg_token) {SPACE, " "},
		(msg_token) {PARAM, "0"},
		(msg_token) {SPACE, " "},
		(msg_token) {PARAM, "*"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":Álvaro Martínez"},
		(msg_token) {CRLF, "\r\n"}
	};
	print_part_test(8, msgTokenizer(packet), testT);
	std::cout << "---------------------------------\n";

	// /* Test 9 */
	packet = "JOIN #a,#b,#c keyA,keyB,keyC\r\n";
	testT = {
		(msg_token) {WORD, "JOIN"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "#a,#b,#c"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "keyA,keyB,keyC"},
		(msg_token) {CRLF, "\r\n"}
	};
	print_part_test(9, msgTokenizer(packet), testT);
	std::cout << "---------------------------------\n";

	/*Test 10 */
	packet = "MODE #a,+i,#b,-t\r\n";
	testT = {
		(msg_token) {WORD, "MODE"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "#a,+i,#b,-t"},
		(msg_token) {CRLF, "\r\n"}
	};
	print_part_test(10, msgTokenizer(packet), testT);

	tokens = msgTokenizer(packet);
	status = VALID_MSG;
	in = parseMessage(tokens, status);
	status = VALID_MSG;
	std::cout << g_parseErrors[status] << std::endl;
	assert(status == VALID_MSG);
	assert(in.cmd == MODE);
	std::cout << "---------------------------------\n";

	/* Test 11 */
	packet = ":!@ PRIVMSG #canal :prefijo vacío\r\n";
	testT = {
		(msg_token) {PREFIX, ":!@"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":prefijo vacío"},
		(msg_token) {CRLF, "\r\n"}
	};
	print_part_test(11, msgTokenizer(packet), testT);

	tokens = msgTokenizer(packet);
	status = VALID_MSG;
	in = parseMessage(tokens, status);
	std::cout << g_parseErrors[status] << std::endl;
	assert(status != VALID_MSG);
	assert(in.cmd == COMMAND0);
	std::cout << "---------------------------------\n";

	/* Test 12 */
	packet = ":Nick!user@host@ PRIVMSG #canal :prefijo inválido\r\n";
	testT = {
		(msg_token) {PREFIX, ":Nick!user@host@"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, ":prefijo inválido"},
		(msg_token) {CRLF, "\r\n"}
	};
	print_part_test(12, msgTokenizer(packet), testT);

	tokens = msgTokenizer(packet);
	status = VALID_MSG;
	in = parseMessage(tokens, status);
	std::cout << g_parseErrors[status] << std::endl;
	assert(status != VALID_MSG);
	assert(in.cmd == COMMAND0);
	std::cout << "---------------------------------\n";
}