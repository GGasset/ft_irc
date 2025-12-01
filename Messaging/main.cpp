

// int main(void) {
//     Server server;
//     MessageIn in;
//     User      senderU;

//     init_server(server, "1234");
    
//     std::string packet = "PASS 1234\r\n";

//     if (!prepare_message(packet, server, in))
//         return 0;

//     handle_message(in, server, packet);

//     packet = "NICK rata\r\n";

//     if (!prepare_message(packet, server, in))
//         return 0;

//     handle_message(in, server, packet);

//     packet = "USER Rata 0 * :ratata\r\n";

//     if (!prepare_message(packet, server, in))
//         return 0;

//     handle_message(in, server, packet);

//     packet = "PING rata\r\n";
    
//     if (!prepare_message(packet, server, in))
//         return 0;

//     handle_message(in, server, packet);
// }
