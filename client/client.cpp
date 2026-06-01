#include <iostream>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

int main()
{
    const int PORT = 8080;

    int sock =
        socket(AF_INET,
               SOCK_STREAM,
               0);

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET,
              "127.0.0.1",
              &server_addr.sin_addr);

    if(connect(sock,
               (sockaddr*)&server_addr,
               sizeof(server_addr)) < 0)
    {
        cout << "Connection failed\n";
        return 1;
    }

    cout << "Connected!\n";

    string message;

    while(true)
    {
        getline(cin, message);

        if(message == "/quit")
            break;

        send(sock,
             message.c_str(),
             message.size(),
             0);
    }

    close(sock);

    return 0;
}