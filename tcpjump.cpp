//
// Created by lsc on 24-6-11.
//
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

std::vector<int> backend_ports;
std::atomic<size_t> next_backend(0);
std::mutex cout_mutex;

void forward_connection(int client_sock, int backend_port) {
    int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_sock < 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to create backend socket: " << strerror(errno) << std::endl;
        close(client_sock);
        return;
    }

    sockaddr_in backend_addr;
    memset(&backend_addr, 0, sizeof(backend_addr));
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend_port);
    inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr);

    if (connect(backend_sock, (sockaddr*)&backend_addr, sizeof(backend_addr)) < 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to connect to backend port " << backend_port << ": " << strerror(errno) << std::endl;
        close(client_sock);
        close(backend_sock);
        return;
    }

    std::thread([client_sock, backend_sock]() {
        char buffer[4096];
        ssize_t n;

        while ((n = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
            send(backend_sock, buffer, n, 0);
        }

        close(client_sock);
        close(backend_sock);
    }).detach();

    std::thread([client_sock, backend_sock]() {
        char buffer[4096];
        ssize_t n;

        while ((n = recv(backend_sock, buffer, sizeof(buffer), 0)) > 0) {
            send(client_sock, buffer, n, 0);
        }

        close(client_sock);
        close(backend_sock);
    }).detach();
}

void start_forwarder(int listen_port) {
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        std::cerr << "Failed to create listen socket: " << strerror(errno) << std::endl;
        return;
    }

    sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(listen_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_sock, (sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        std::cerr << "Failed to bind to port " << listen_port << ": " << strerror(errno) << std::endl;
        close(listen_sock);
        return;
    }

    if (listen(listen_sock, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on port " << listen_port << ": " << strerror(errno) << std::endl;
        close(listen_sock);
        return;
    }

    std::cout << "Listening on port " << listen_port << std::endl;

    while (true) {
        int client_sock = accept(listen_sock, nullptr, nullptr);
        if (client_sock < 0) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        size_t backend_index = next_backend++ % backend_ports.size();
        int backend_port = backend_ports[backend_index];

        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Forwarding connection to backend port " << backend_port << std::endl;

        std::thread(forward_connection, client_sock, backend_port).detach();
    }

    close(listen_sock);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <listen_port> <backend_start_port> <backend_end_port>" << std::endl;
        return EXIT_FAILURE;
    }

    int listen_port = std::stoi(argv[1]);
    int backend_start_port = std::stoi(argv[2]);
    int backend_end_port = std::stoi(argv[3]);

    for (int port = backend_start_port; port <= backend_end_port; ++port) {
        backend_ports.push_back(port);
    }

    start_forwarder(listen_port);

    return EXIT_SUCCESS;
}