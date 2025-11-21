#include "ParserMessage.hpp"

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

void	test_validity(size_t id, std::string packet, msgTokens testT, ParseStatus testStatus, COMMAND testCommand)
{
	msgTokens	tokens;
	ParseStatus	status = VALID_MSG;
	MessageIn	in;

	tokens = msgTokenizer(packet);
	print_part_test(id, tokens, testT);
	status = VALID_MSG;
	in = parseMessage(tokens, status);
	std::cout << g_parseErrors[status] << std::endl;
	std::cout << "[status]: " << status << " [testStatus]: " << testStatus << std::endl;
	std::cout << "[cmd]: " << in.getCommand() << " [testCmd]: " << testCommand << std::endl;
	assert(status == testStatus);
	assert(in.getCommand() == testCommand);
	std::cout << "---------------------------------\n";
}


int main(void)
{
	std::string packet;
	msgs		packet_msgs;
	msgs		test;
	msgTokens	testT;

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
		(msg_token) {TOK_PARAM, "alvaro"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "0"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "*"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, "Álvaro Martínez"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(8, packet, testT, VALID_MSG, USER);

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
	test_validity(9, packet, testT, VALID_MSG, JOIN);

	/*Test 10 */
	packet = "MODE #a,+i,#b,-t\r\n";
	testT = {
		(msg_token) {WORD, "MODE"},
		(msg_token) {SPACE, " "},
		(msg_token) {COMMA_LIST, "#a,+i,#b,-t"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(10, packet, testT, VALID_MSG, MODE);

	/* Test 11 */
	packet = ":!@ PRIVMSG #canal :prefijo vacío\r\n";
	testT = {
		(msg_token) {PREFIX, ":!@"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, "prefijo vacío"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(11, packet, testT, PERR_PREFIX_MISSING_NICK, COMMAND0);

	/* Test 12 */
	packet = ":Nick!user@host@ PRIVMSG #canal :prefijo inválido\r\n";
	testT = {
		(msg_token) {PREFIX, ":Nick!user@host@"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, "prefijo inválido"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(12, packet, testT, PERR_PREFIX_INVALID_HOST, COMMAND0);
	
	/* Test 13 */
	packet = ":Nick!us er@host PRIVMSG #canal :prefijo inválido\r\n";
	testT = {
		(msg_token) {PREFIX, ":Nick!us"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "ER@HOST"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, "prefijo inválido"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(13, packet, testT, PERR_PREFIX_MISSING_HOST, COMMAND0);

	/* Test 14 */
	packet = ":Nick@us er@host PRIVMSG #canal :prefijo inválido\r\n";
	testT = {
		(msg_token) {PREFIX, ":Nick@us"},
		(msg_token) {SPACE, " "},
		(msg_token) {WORD, "ER@HOST"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "PRIVMSG"},
		(msg_token) {SPACE, " "},
		(msg_token) {TOK_PARAM, "#canal"},
		(msg_token) {SPACE, " "},
		(msg_token) {TRAIL, "prefijo inválido"},
		(msg_token) {CRLF, "\r\n"}
	};
	test_validity(14, packet, testT, PERR_PREFIX_MISSING_USER, COMMAND0);

			/*				Casos Easy				  */	

	/* Test 15 — NICK simple */
	packet = "NICK Alvaro\r\n";
	testT = {
		{WORD, "NICK"}, {SPACE, " "}, {TOK_PARAM, "Alvaro"}, {CRLF, "\r\n"}
	};
	test_validity(15, packet, testT, VALID_MSG, NICK);

	size_t idx = 16;
	/* Test 16 — PONG con trailing */
	packet = "PONG :irc.local\r\n";
	testT = {
		{WORD, "PONG"}, {SPACE, " "}, {TRAIL, "irc.local"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, PONG);

	/* Test 16 — QUIT con mensaje */
	packet = "QUIT :Hasta luego\r\n";
	testT = {
		{WORD, "QUIT"}, {SPACE, " "}, {TRAIL, "Hasta luego"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, QUIT);

	/* Test 18 — JOIN sin key */
	packet = "JOIN #literatura\r\n";
	testT = {
		{WORD, "JOIN"}, {SPACE, " "}, {TOK_PARAM, "#literatura"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, JOIN);

			/*				Canales y Claves			  */	
			
	/* Test 19 */
	packet = "JOIN #a,#b,#c keyA,keyB\r\n";
	testT = {
		{WORD, "JOIN"}, {SPACE, " "}, {COMMA_LIST, "#a,#b,#c"}, {SPACE, " "},
		{COMMA_LIST, "keyA,keyB"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, JOIN);

	/* Test 20 — PRIVMSG a múltiples nicks */
	packet = "PRIVMSG Alvaro,Aleister,Ramon :Saludos\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {COMMA_LIST, "Alvaro,Aleister,Ramon"},
		{SPACE, " "}, {TRAIL, "Saludos"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 21 — MODE con argumentos mixtos */
	packet = "MODE #canal +ov Alvaro,Belen\r\n";
	testT = {
		{WORD, "MODE"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TOK_PARAM, "+ov"}, {SPACE, " "}, {COMMA_LIST, "Alvaro,Belen"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, MODE);

		/* Test 22 — Prefijo con solo nick */
	packet = ":Ramon PRIVMSG #literatura :Hola\r\n";
	testT = {
		{PREFIX, ":Ramon"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#literatura"}, {SPACE, " "}, {TRAIL, "Hola"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 23 — Prefijo con nick y user */
	packet = ":Ramon!lector PRIVMSG #libros :Buenos días\r\n";
	testT = {
		{PREFIX, ":Ramon!lector"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#libros"}, {SPACE, " "}, {TRAIL, "Buenos días"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, PERR_PREFIX_MISSING_HOST, COMMAND0);

	/* Test 24 — Prefijo con nick, user, host */
	packet = ":Ramon!lector@biblioteca PRIVMSG #libros :¡Hola!\r\n";
	testT = {
		{PREFIX, ":Ramon!lector@biblioteca"}, {SPACE, " "}, {WORD, "PRIVMSG"},
		{SPACE, " "}, {TOK_PARAM, "#libros"}, {SPACE, " "}, {TRAIL, "¡Hola!"},
		{CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 25 — Prefijo servidor */
	packet = ":irc.local 001 Alvaro :Bienvenido\r\n";
	testT = {
		{PREFIX, ":irc.local"}, {SPACE, " "}, {NUMBER, "001"}, {SPACE, " "},
		{TOK_PARAM, "Alvaro"}, {SPACE, " "}, {TRAIL, "Bienvenido"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, COMMAND0);

		/* Test 26 — Prefijo vacío con solo ':' */
	packet = ": PRIVMSG #canal :mensaje\r\n";
	testT = {
		{PREFIX, ":"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#canal"}, {SPACE, " "}, {TRAIL, "mensaje"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, PERR_PREFIX_INVALID_SERVERNAME, COMMAND0);

	/* Test 27 — Comando numérico */
	packet = ":irc.local 376 Alvaro :End of MOTD\r\n";
	testT = {
		{PREFIX, ":irc.local"}, {SPACE, " "}, {NUMBER, "376"}, {SPACE, " "},
		{TOK_PARAM, "Alvaro"}, {SPACE, " "}, {TRAIL, "End of MOTD"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, COMMAND0);

	/* Test 28 — Comando en minúsculas */
	packet = "join #literatura\r\n";
	testT = {
		{WORD, "JOIN"}, {SPACE, " "}, {TOK_PARAM, "#literatura"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, VALID_MSG, JOIN);

	/* Test 29 — Prefijo largo límite (50 chars) */
	packet = ":aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa PRIVMSG #a :msg\r\n";
	testT = {
		{PREFIX, ":aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
		{SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#a"}, {SPACE, " "},
		{TRAIL, "msg"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, PERR_PREFIX_LENGTH, COMMAND0);

	/* Test 30 — Nickname con caracteres válidos */
	packet = ":A[]\\`^{}_- PRIVMSG #canal :chars válidos\r\n";
	testT = {
		{PREFIX, ":A[]\\`^{}_-"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#canal"}, {SPACE, " "}, {TRAIL, "chars válidos"}, {CRLF, "\r\n"}
	};
	test_validity(idx++, packet, testT, PERR_PREFIX_INVALID_SERVERNAME, COMMAND0);
	
	/* Test 31 */
	packet = ":A[]!\\^@{}_- PRIVMSG #canal :chars válidos\r\n";
	testT = {
		{PREFIX, ":A[]!\\^@{}_-"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#canal"}, {SPACE, " "}, {TRAIL, "chars válidos"}, {CRLF, "\r\n"}
	};
	// test_validity(idx++, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 32 Prefijo doble @addtogroup*/
	packet = ":Nick!user@host@ PRIVMSG #canal :doble arroba\r\n";
	testT = {
		{PREFIX, ":Nick!user@host@"}, {SPACE, " "}, {WORD, "PRIVMSG"}, {SPACE, " "},
		{TOK_PARAM, "#canal"}, {SPACE, " "}, {TRAIL, "doble arroba"}, {CRLF, "\r\n"}
	};
	test_validity(32, packet, testT, PERR_PREFIX_INVALID_HOST, COMMAND0);

	/* Test 33 -- Esta la entiende como nick invalido, pero para mi es comando invalido.*/
	packet = ":Ni ck!user@host PRIVMSG #canal :nick inválido\r\n";
	testT = {
		{PREFIX, ":Ni"}, {SPACE, " "}, {WORD, "CK!USER@HOST"}, {SPACE, " "},
		{TOK_PARAM, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, "nick inválido"}, {CRLF, "\r\n"}
	};
	test_validity(33, packet, testT, PERR_INVALID_COMMAND, COMMAND0);

	/* Test 34  Válido*/
	packet = "PRIVMSG #canal :hola:adiós\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, "hola:adiós"}, {CRLF, "\r\n"}
	};
	test_validity(34, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 35 */
	packet = "PRIVMSG #canal :\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, ""}, {CRLF, "\r\n"}
	};
	test_validity(35, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 36 */
	packet = "PRIVMSG    #canal    :hola\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, "hola"}, {CRLF, "\r\n"}
	};
	test_validity(36, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 37 */
	packet = "TOPIC #a :literatura gótica \r\n";
	testT = {
		{WORD, "TOPIC"}, {SPACE, " "}, {TOK_PARAM, "#a"}, {SPACE, " "},
		{TRAIL, "literatura gótica "}, {CRLF, "\r\n"}
	};
	test_validity(37, packet, testT, VALID_MSG, TOPIC);

	/* Test 38 */
	packet = "PRIVMSG #canal :hola\x07\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, "hola\x07"}, {CRLF, "\r\n"}
	};
	test_validity(38, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 39 */
	packet = "PRIVMSG #canal :mañana ☀️\r\n";
	testT = {
		{WORD, "PRIVMSG"}, {SPACE, " "}, {TOK_PARAM, "#canal"}, {SPACE, " "},
		{TRAIL, "mañana ☀️"}, {CRLF, "\r\n"}
	};
	test_validity(39, packet, testT, VALID_MSG, PRIVMSG);

	/* Test 40 */
	packet = "RATON #canal :buenos dias\r\n";
	testT = {
		{WORD, "RATON"}, {SPACE, " "}, {TOK_PARAM, "#canal"},{SPACE, " "},
		{TRAIL, "buenos dias"}, {CRLF, "\r\n"}
	};
	test_validity(40, packet, testT, PERR_INVALID_COMMAND, COMMAND0);

	/* Test 41 */
	packet = "\r\n";
	testT = {
		{CRLF, "\r\n"}
	};
	test_validity(41, packet, testT, VALID_MSG, COMMAND0);

	/* Test 42 */
	packet = " \r\n";
	testT = {
		{SPACE, " "}, {CRLF, "\r\n"}
	};
	test_validity(42, packet, testT, PERR_MISSING_COMMAND, COMMAND0);
}

