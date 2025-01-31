#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

// Structure to store client information
struct Client {
    int id;
    std::string name;
    int socket;
};

// Global variables
std::vector<Client> clients;
std::mutex clients_mtx; // To protect access to the clients vector
int seed = 0;

// Broadcast message to all connected clients
void broadcast_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mtx);
    for (const auto& client : clients) {
        send(client.socket, message.c_str(), message.size(), 0);
    }
}

// Handle communication with a client
void handle_client(Client client) {
    char buffer[1024];
    std::string welcome_message = client.name + " has joined the chat.\n";
    broadcast_message(welcome_message);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client.socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            // Remove client and notify others
            {
                std::lock_guard<std::mutex> lock(clients_mtx);
                clients.erase(std::remove_if(clients.begin(), clients.end(),
                                             [client](const Client& c) { return c.socket == client.socket; }),
                              clients.end());
            }
            close(client.socket);
            std::string leave_message = client.name + " has left the chat.\n";
            broadcast_message(leave_message);
            break;
        }

        // Broadcast received message
        std::string message = client.name + ": " + std::string(buffer);
        broadcast_message(message);
    }
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // Port number
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }

    // Listen for connections
    if (listen(server_socket, 10) == -1) {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }
    std::cout << "Server is running on port 8080..." << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Accept a new client
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        // Receive the client's name
        char name_buffer[1024];
        memset(name_buffer, 0, sizeof(name_buffer));
        recv(client_socket, name_buffer, sizeof(name_buffer) - 1, 0);
        std::string client_name(name_buffer);

        // Generate a new client ID
        seed++;
        Client client{seed, client_name, client_socket};

        {
            std::lock_guard<std::mutex> lock(clients_mtx);
            clients.push_back(client);
        }

        // Create a thread to handle the client
        std::thread(handle_client, client).detach();
    }

    // Close server socket
    close(server_socket);
    return 0;
}