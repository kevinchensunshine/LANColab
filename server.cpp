#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <charconv>
#include <atomic>
#include <thread>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

 // Need to link with Ws2_32.lib, Mswsock.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using SOCKET = int;
constexpr SOCKET INVALID_SOCKET = -1;

#endif

constexpr uint16_t LISTENING_PORT_DEFAULT = 5000;
constexpr uint16_t LISTENING_PORT_MIN = 1024;
constexpr int LISTENING_QUEUE_LENGTH = 5;
constexpr size_t BUFFER_INITIAL_SIZE = 1024 * 1024;
constexpr size_t MAX_CONNECTED_CLIENTS = 32;

std::atomic_size_t g_num_connected_clients = 0;

/* receive specified amount of data from a stream socket */
static size_t receive_data(SOCKET s, uint8_t *buffer, size_t length, struct timeval *timeout)
{
    size_t received = 0;

    while (received < length)
    {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(s, &rfds);

        int ret = select((int)s + 1, &rfds, NULL, NULL, timeout);

        if (ret > 0)
        {
            int bytes_read = recv(s, (char *)(buffer + received), (int)(length - received), 0);
            if (bytes_read > 0)
            {
                received += bytes_read;
            }
            else
            {
                break;
            }
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            break;
        }
    }

    return received;
}

/* serve a connected client in its own thread */
static void serve_client(SOCKET s, std::string client_ip_addr, uint16_t client_port)
{
    // allocate a fixed sized buffer that is large enough
    std::vector<uint8_t> buffer(BUFFER_INITIAL_SIZE);

    while (1)
    {
        int bytes_read = recv(s, (char *)(buffer.data()), (int)(buffer.size()), 0);
        if (bytes_read > 0)
        {
            if (write(s, (char *)(buffer.data()), bytes_read) != bytes_read) {
                std::cout << "Error writing from bytes read \n";
            }
        }
        else
        {
            std::cout << " Connection reset by client \n";
            break;
        }
    }

#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif

    g_num_connected_clients--;
}

int main(int argc, char *argv[])
{
    uint16_t listening_port = LISTENING_PORT_DEFAULT;

    if (argc > 1)
    {
        std::string arg(argv[1]);
        auto[p, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), listening_port);
        if (ec != std::errc())
        {
            std::cerr << "Invalid listening port argument " << arg << "\n";
            return EXIT_FAILURE;
        }

        if (listening_port < LISTENING_PORT_MIN)
        {
            std::cerr << "Listening port out of range (" << LISTENING_PORT_MIN << " - " << (UINT16_MAX - 1) << ")\n";
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " <listening_port>\n";
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    WSADATA wsaData;

    // winsock required initialization
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        std::cerr << "WSAStartup() failed.\n";
        return EXIT_FAILURE;
    }
#endif

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == INVALID_SOCKET)
    {
        std::cerr << "Couldn't create socket.\n";
        return EXIT_FAILURE;
    }

    int n = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&n, sizeof(n)) != 0)
    {
        std::cerr << "setsockopt(,,SO_REUSERADDR,,) failed.\n";
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listening_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Couldn't bind socket to port\n";
        return EXIT_FAILURE;
    }

    if (listen(s, LISTENING_QUEUE_LENGTH) < 0)
    {
        std::cerr << "Couldn't listen on socket.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Listening on port " << listening_port << "\n";

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len;

        addr_len = sizeof(client_addr);

        SOCKET client_sock = accept(s, (struct sockaddr *)&client_addr, &addr_len);

        if (client_sock != INVALID_SOCKET)
        {
#ifdef _WIN32
            char client_ip_addr_buffer[16];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_addr_buffer, sizeof(client_ip_addr_buffer));
            std::string client_ip_addr(client_ip_addr_buffer);
#else
            std::string client_ip_addr(inet_ntoa(client_addr.sin_addr));
#endif

            std::cout << "Accepted connection from client " << client_ip_addr << ":" << ntohs(client_addr.sin_port) << "\n";

            if (g_num_connected_clients < MAX_CONNECTED_CLIENTS)
            {
                g_num_connected_clients++;
                auto thread_handle = std::thread(serve_client, client_sock, client_ip_addr, client_addr.sin_port);
                thread_handle.detach(); // the client service thread runs independently from the main thread
            }
            else
            {
                std::cout << "Closing connection from client " << client_ip_addr << ":" << client_addr.sin_port
                    << " due to too many connections\n";
#ifdef _WIN32
                closesocket(client_sock);
#else
                close(client_sock);
#endif
            }
        }
    }

#ifdef _WIN32
    closesocket(s);
    WSACleanup();
#else
    close(s);
#endif

    return EXIT_SUCCESS;
}
