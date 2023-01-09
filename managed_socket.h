#ifndef MANAGEDSOCKET_HPP
#define MANAGEDSOCKET_HPP

#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

/*Class to manage socket within RAII. 
Otherwise, close(socket) calls would be 
required for all error handles*/

class managed_socket {
    int sock = 0;
public:
    managed_socket() = default;

    managed_socket(int __domain, int __type, int __protocol) {
        sock = socket(__domain, __type, __protocol);
    }

    ~managed_socket() {
        if (sock > 0) {
            close(sock);
        }
    }

    int get_socket() {
        return sock;
    }
};

#endif