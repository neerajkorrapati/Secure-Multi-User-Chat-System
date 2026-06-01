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
    int sock =
        socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET,
              "127.0.0.1",
              &server_addr.sin_addr);

    // Connect to server
    if(connect(sock,
               (sockaddr*)&server_addr,
               sizeof(server_addr)) < 0)
    {
        cout << "Connection failed\n";
        return 1;
    }

    const char* message =
        "Hello Server";

    // Send message
    send(sock,
         message,
         strlen(message),
         0);
    char buffer[1024];

    // Receive reply
    int bytes =
        recv(sock,buffer,sizeof(buffer),0);
    if(bytes > 0)
    {
        cout << "Server replied: ";
        cout.write(buffer, bytes);
        cout << endl;
    }

    close(sock);

    return 0;
}