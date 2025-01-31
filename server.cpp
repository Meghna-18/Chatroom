#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>

#define MAX_LEN 200

// Global variable to store the client socket
int client_socket;
std::mutex mtx;  // To handle synchronized output to the console

// Function to handle sending messages to the server
void send_message() {
    char message[MAX_LEN];
    while (true) {
        std::cout << "You: ";
        std::cin.getline(message, MAX_LEN);

        if (strcmp(message, "#exit") == 0) {
            send(client_socket, message, strlen(message), 0);  // Send exit message
            close(client_socket);  // Close connection to server
            break;
        }

        send(client_socket, message, strlen(message), 0);  // Send message to server
    }
}

// Function to handle receiving messages from the server
void receive_message() {
    char message[MAX_LEN];
    while (true) {
        memset(message, 0, sizeof(message));
        int bytes_received = recv(client_socket, message, sizeof(message) - 1, 0);

        if (bytes_received <= 0) {
            std::cout << "Disconnected from server\n";
            close(client_socket);
            break;
        }

        std::lock_guard<std::mutex> lock(mtx);  // Lock the console output to avoid garbled output
        std::cout << message << std::endl;  // Display received message
    }
}

int main() {
    std::string server_ip = "127.0.0.1";  // Server IP (localhost)
    int server_port = 8080;  // Server port

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up the server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());  // Convert IP address to binary

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        return -1;
    }

    // Ask for the user's name
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    send(client_socket, name.c_str(), name.length(), 0);  // Send the name to the server

    // Create threads for sending and receiving messages
    std::thread send_thread(send_message);
    std::thread receive_thread(receive_message);

    // Wait for threads to finish
    send_thread.join();
    receive_thread.join();

    return 0;
}