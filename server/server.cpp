#include <iostream>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

int main()
{
    const int PORT = 8080;
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0)
    {
        cout << "Socket creation failed\n";
        return 1;
    }
    // Configure address
    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if(bind(server_fd,
            (sockaddr*)&server_addr,
            sizeof(server_addr)) < 0)
    {
        cout << "Bind failed\n";
        return 1;
    }
    // Listen for connections
    listen(server_fd, 5);
    cout << "Server listening on port "
         << PORT << endl;

    // Accept client
    int client_fd =
        accept(server_fd,
               nullptr,
               nullptr);
    cout << "Client connected\n";
    // Receive message
    char buffer[1024];
    int bytes = recv(client_fd,buffer,sizeof(buffer),0);
    if(bytes > 0)
    {
        cout << "Client says: ";
        cout.write(buffer, bytes);
        cout << endl;
    }
    // Send reply
    const char* reply =
        "Hello from server";
    send(client_fd,
         reply,
         strlen(reply),
         0);

    close(client_fd);
    close(server_fd);
    return 0;
}