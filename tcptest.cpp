//
// Created by lsc on 24-6-11.
//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

void handle_client(int client_sock) {
    char buffer[1024];
    ssize_t n;

    while ((n = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        send(client_sock, buffer, n, 0); // Echo the message back to the client
    }

    close(client_sock);
}

void start_server(int port) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind to port " << port << ": " << strerror(errno) << std::endl;
        close(server_sock);
        return;
    }

    if (listen(server_sock, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on port " << port << ": " << strerror(errno) << std::endl;
        close(server_sock);
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock < 0) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        std::thread(handle_client, client_sock).detach();
    }

    close(server_sock);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[1]);

    start_server(port);

    return EXIT_SUCCESS;
}