# Updates

Included a naive database to extend the server network to full stack. Currently, the two have not been connected, but the database has been tested individually to effectively store data relationally with dynamic columns. It will be able to be queried from the server in the future and retrieve data by ID. Hopefully, this can be used to create some interesting low-level projects that will further my understanding of high-level computer science concepts

# Primitive Server

Primitive Server contains my ongoing learning about network programming through the creation of a functional server and client. It runs simply by providing the same port and the host that the server is on through the command line, and can take up to 32 clients. Recently, I've used to in combination with a seeded random number generator to create a phrase of the day along with an echo response.

# Usage

Download the files and compile with:
g++ -std=c++17 server.cpp -o server\
Usage requires C++17 due to specific syntax related to pair indexing\
On the client:
g++ -std=c++17 client.cpp -o client\
To start the server, run the executable and provide a port: most ports afer 1024 are often free.
To start the client, run the executable and provide the IP of the server and the port.

Ex:
./server 5000
./client 127.0.0.1 5000
