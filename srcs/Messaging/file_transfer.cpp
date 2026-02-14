// #include "Server.hpp"
// #include "File.hpp"
// #include "User.hpp"

// // Envía el siguiente chunk de la transferencia de archivo
// void process_file_transfers(Server &server)
// {
// 	// Este método debería ser llamado periódicamente desde el server loop
// 	// Procesa todas las transferencias activas
// }

// // Inicia una transferencia de archivo
// void SEND_fn(command_args args, Server& server, User& sender)
// {
// 	if (args.argv.size() < 3)
// 	{
// 		server.add_msg(RED"461 SEND :Not enough parameters" RESET, sender);
// 		return;
// 	}

// 	// SEND <target_nick> <filename>
// 	std::string target_nick = args.argv[1];
// 	std::string file_name = args.argv[2];

// 	// Validar que el archivo existe
// 	std::ifstream test_file(file_name.c_str());
// 	if (!test_file.is_open())
// 	{
// 		server.add_msg(RED"NOTICE " + sender.get_nick() + " :File not found: " + file_name + RESET, sender);
// 		return;
// 	}
// 	test_file.close();

// 	// Buscar al usuario destino
// 	User *target_user = server.get_user_by_nick(target_nick);
// 	if (!target_user)
// 	{
// 		server.add_msg(RED"401 " + target_nick + " :No such nick" RESET, sender);
// 		return;
// 	}

// 	// Crear objeto File y guardarlo en el servidor
// 	File transfer(file_name, file_name, sender.get_nick(), target_nick);
	
// 	if (!transfer.open_file())
// 	{
// 		server.add_msg(RED"NOTICE " + sender.get_nick() + " :Cannot open file: " + file_name + RESET, sender);
// 		return;
// 	}

// 	// Notificar al receptor sobre la transferencia
// 	server.add_msg(YELLOW"NOTICE " + target_nick + " :Incoming file transfer from " + sender.get_nick() 
// 		+ ": " + file_name + " (" + std::to_string(transfer.get_file_size()) + " bytes)" RESET, *target_user);

// 	server.add_msg(GREEN"NOTICE " + sender.get_nick() + " :File transfer started to " + target_nick 
// 		+ ": " + file_name + RESET, sender);

// 	// Comenzar a enviar chunks
// 	std::string chunk;
// 	size_t chunk_count = 0;
// 	while (!transfer.is_complete())
// 	{
// 		chunk = transfer.read_chunk();
// 		if (chunk.empty())
// 			break;
		
// 		// Enviar cada chunk como un mensaje FT (File Transfer)
// 		// Formato: :nick FT <sender> <filename> <sequence> <data>
// 		server.add_msg(":ft_" + std::to_string(chunk_count) + " FT " + sender.get_nick() 
// 			+ " " + file_name + " " + std::to_string(chunk_count) 
// 			+ " :" + chunk, *target_user);
		
// 		chunk_count++;
// 	}

// 	// Enviar mensaje de finalización
// 	server.add_msg(GREEN"NOTICE " + target_nick + " :File transfer complete from " + sender.get_nick() 
// 		+ " - " + file_name + " (" + std::to_string(transfer.bytes_sent) + " bytes)" RESET, *target_user);

// 	server.add_msg(GREEN"NOTICE " + sender.get_nick() + " :File " + file_name 
// 		+ " sent successfully to " + target_nick + RESET, sender);

// 	transfer.close_file();
// }

// // Acepta una transferencia de archivo
// void ACCEPT_fn(command_args args, Server& server, User& sender)
// {
// 	if (args.argv.size() < 2)
// 	{
// 		server.add_msg(RED"461 ACCEPT :Not enough parameters" RESET, sender);
// 		return;
// 	}

// 	std::string sender_nick = args.argv[1];
// 	std::string save_path = (args.argv.size() > 2) ? args.argv[2] : sender_nick + "_received_file";

// 	server.add_msg(GREEN"NOTICE " + sender.get_nick() + " :Ready to receive file from " + sender_nick 
// 		+ " and save as: " + save_path + RESET, sender);
// }

// // Rechaza una transferencia de archivo
// void REFUSE_fn(command_args args, Server& server, User& sender)
// {
// 	if (args.argv.size() < 2)
// 	{
// 		server.add_msg(RED"461 REFUSE :Not enough parameters" RESET, sender);
// 		return;
// 	}

// 	std::string sender_nick = args.argv[1];
// 	User *target = server.get_user_by_nick(sender_nick);
// 	if (target)
// 	{
// 		server.add_msg(RED"NOTICE " + sender_nick + " :" + sender.get_nick() 
// 			+ " refused the file transfer" RESET, *target);
// 	}

// 	server.add_msg(YELLOW"NOTICE " + sender.get_nick() + " :File transfer refused" RESET, sender);
// }
