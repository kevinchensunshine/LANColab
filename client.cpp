#include <iostream>
#include <charconv>
#include <vector>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

 // Need to link with Ws2_32.lib, Mswsock.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#else
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "managed_socket.h"

using SOCKET = int;
constexpr SOCKET INVALID_SOCKET = -1;
constexpr uint16_t LISTENING_PORT_DEFAULT = 5000;

#endif

#define IBUFSIZE 1024

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host>, <port>\n";
        return EXIT_FAILURE;
    } 
    std::string arg(argv[2]);

    //takes the port info from the command line and initializes server_port with it. Returns a pair for error checking
    uint16_t server_port = LISTENING_PORT_DEFAULT;
    auto[p, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), server_port);
    if (ec != std::errc())
    {
        std::cerr << "Invalid listening port argument " << arg << "\n";
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    managed_socket sock(AF_INET, SOCK_STREAM, 0);

    if ((sock.get_socket()) < 0) {
        std::cerr << "\nSocket Creation Error\n";
        return -1;
    }

    //Set parameters to the IPv4 family. Do not remove htons for the sake of portability
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    in_addr addr;
    if (inet_aton(argv[1], &addr) == 0) {
        std::cerr << "\n Failed to Convert Host IP to inet \n";
        return EXIT_FAILURE;
    }
    server_addr.sin_addr = addr;

    if (connect(sock.get_socket(), (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "\n Could not connect to server " << strerror(errno) << "\n";
        return EXIT_FAILURE;
    } else {
        std::cout << "\n Successfully Connected to Server \n";
    }

    std::cout << "\n Type anything you want. Press '#' to escape \n";
    
    int read, sent = 0;
    std::string in_buf;
    std::vector<uint8_t> tlv_buffer(IBUFSIZE);

    while (1) {
        getline(std::cin, in_buf);

        if (in_buf == "#") {
            std::cout << "\n Exited the Client\n ";
            break;
        }

        const char* input = in_buf.c_str();
        size_t len = in_buf.length();

        if (write(sock.get_socket(), input, len) == -1) {
            std::cout << "Error writing from input read \n";
            break;
        }

        int bytes_read = recv(sock.get_socket(), (char *)(tlv_buffer.data()), (int)(tlv_buffer.size()), 0);
        
        if (bytes_read > 0) {
            std::string output((char*)tlv_buffer.data(), bytes_read); 
            std::cout << output << "\n";
            continue;
        } else {
            std::cout << "Could not read from server \n";
            break;
        }
    }

    return EXIT_SUCCESS;
}