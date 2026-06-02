#include <iostream>
#include <vector>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

using namespace std;

int main()
{
    const int PORT = 8080;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        cout << "Socket creation failed\n";
        return 1;
    }
    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(bind(server_fd,
            (sockaddr*)&server_addr,
            sizeof(server_addr)) < 0)
    {
        cout << "Bind failed\n";
        return 1;
    }

    listen(server_fd, 10);

    vector<int> clients;

    cout << "Server listening on port "
         << PORT << endl;

    while(true)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(server_fd, &readfds);

        int maxfd = server_fd;

        for(int client : clients)
        {
            FD_SET(client, &readfds);

            if(client > maxfd)
                maxfd = client;
        }

        int activity =
            select(maxfd + 1,
                   &readfds,
                   nullptr,
                   nullptr,
                   nullptr);

        if(activity < 0)
            continue;

        // New connection
        if(FD_ISSET(server_fd, &readfds))
        {
            int new_client =
                accept(server_fd,
                       nullptr,
                       nullptr);

            clients.push_back(new_client);

            cout << "[+] Client "
                 << new_client
                 << " connected\n";
        }

        char buffer[1024];

        for(auto it = clients.begin();
            it != clients.end();)
        {
            int client = *it;

            if(FD_ISSET(client, &readfds))
            {
                int bytes =
                    recv(client,
                         buffer,
                         sizeof(buffer),
                         0);

                if(bytes <= 0)
                {
                    cout << "[-] Client "
                         << client
                         << " disconnected\n";

                    close(client);

                    it = clients.erase(it);

                    continue;
                }
                string message ="Client " +to_string(client) + ": ";
                message.append(buffer, bytes);
                cout << message << endl;

                for(int other :clients){
                    if(other!=client){
                        send(other,message.c_str(),message.size(),0);
                    }
                }
            }

            ++it;
        }
    }

    close(server_fd);

    return 0;
}