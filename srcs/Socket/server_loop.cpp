#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <cstring>
#include <iostream>

#include "Server.hpp"

int signal_server_stop;

/* ===================== SIGNALS ===================== */

void handle_signals(int)
{
    signal_server_stop = true;
}

/* ===================== SOCKET SETUP ===================== */

static int setup_sockfd(size_t PORT)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const sockaddr*)&addr, sizeof(addr)) == -1 ||
        listen(sockfd, 20) == -1)
    {
        close(sockfd);
        return -1;
    }

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    return sockfd;
}

/* ===================== READ EVENT ===================== */

void Server::handle_read_event(int fd)
{
    char tmp[READ_SIZE];

    while (true)
    {
        ssize_t bytes_read = read(fd, tmp, READ_SIZE);

        if (bytes_read > 0)
        {
            ssize_t sender_index = get_user_index_by_fd(fd);
            User *sender = get_user_by_fd(fd);
            if (!sender)
                return;

            std::string read_data(tmp, bytes_read);
            std::vector<std::string> msgs = sender->msg_sent(read_data);

            for (size_t i = 0; i < msgs.size(); i++)
            {
#ifndef DONT_LOG
                std::cout << "\nMsg received from "
                          << sender->getUsername()
                          << ": " << msgs[i] << std::endl;
#endif
                route_message(msgs[i], *sender, sender_index);
            }
        }
        else if (bytes_read == 0)
        {
            // ⚠️ AQUÍ FALTA una función para limpiar correctamente el cliente
            // Debería:
            //  - eliminar fd de epoll
            //  - cerrar fd
            //  - borrar User, messages, client_fds, etc.
            close(fd);
            return;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;

            // ⚠️ Error real: mismo problema que arriba
            close(fd);
            return;
        }
    }
}

/* ===================== WRITE EVENT ===================== */

void Server::handle_write_event(int fd)
{
    ssize_t user_i = get_user_index_by_fd(fd);
    if (user_i == -1)
        return;

    if (messages[user_i].empty())
        return;

    while (!messages[user_i].empty())
    {
        std::tuple<void*, size_t, bool> &msg = messages[user_i].front();
        char *buf = static_cast<char*>(std::get<0>(msg));
        size_t &len = std::get<1>(msg);
        bool free_buf = std::get<2>(msg);

        ssize_t written = write(fd, buf, len);

        if (written > 0)
        {
            if ((size_t)written < len)
            {
                // Escritura parcial
                memmove(buf, buf + written, len - written);
                len -= written;
                return;
            }
            else
            {
                // Mensaje completo
                if (free_buf)
                    delete[] buf;
                messages[user_i].pop();
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;

            // ⚠️ Error real → debería limpiar cliente
            close(fd);
            return;
        }
    }

    // ⚠️ AQUÍ debería desactivarse EPOLLOUT con epoll_ctl(EPOLL_CTL_MOD)
    // pero no existe ninguna función auxiliar para ello en tu código actual
}

/* ===================== EVENT DISPATCH ===================== */

void Server::handle_event(const epoll_event event, int sockfd)
{
    if (event.data.fd == sockfd)
    {
        int new_client_fd = accept(sockfd, 0, 0);
        if (new_client_fd == -1)
            return;

        int flags = fcntl(new_client_fd, F_GETFL, 0);
        fcntl(new_client_fd, F_SETFL, flags | O_NONBLOCK);

#ifndef DONT_LOG
        std::cout << "Client connected fd=" << new_client_fd << std::endl;
#endif

        client_fds.push_back(new_client_fd);
        clients.push_back(User("unset", max_client_id));
        last_pong_time.push_back(std::time(NULL));
        messages.push_back(std::queue<std::tuple<void*, size_t, bool>>());
        max_client_id++;

        epoll_event ev;
        ev.events = EPOLLIN; // ❗ NO EPOLLOUT aquí
        ev.data.fd = new_client_fd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, new_client_fd, &ev);
    }
    else
    {
        if (event.events & EPOLLIN)
            handle_read_event(event.data.fd);
        if (event.events & EPOLLOUT)
            handle_write_event(event.data.fd);
    }
}

/* ===================== MAIN LOOP ===================== */

int Server::loop(size_t PORT)
{
    signal_server_stop = false;
    signal(SIGINT, handle_signals);
    signal(SIGTERM, handle_signals);

    sockfd = setup_sockfd(PORT);
    if (sockfd == -1)
        return 1;

    epollfd = epoll_create1(0);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);

#ifndef DONT_LOG
    std::cout << "Server listening on port " << PORT << std::endl;
#endif

    while (!stop_server && !signal_server_stop)
    {
        int n = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            break;
        }

        for (int i = 0; i < n; i++)
            handle_event(events[i], sockfd);
    }

    close(sockfd);
    close(epollfd);
    for (size_t i = 0; i < client_fds.size(); i++)
        close(client_fds[i]);

    return 0;
}

