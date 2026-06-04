#include <iostream>
#include <vector>
#include<map>
#include<sstream>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

using namespace std;
int findClientByUsername(const map<int,string>& username,const string& name){
    for(auto& pair:username){
       if(pair.second==name){
        return pair.first;
       }
    }
    return -1;
}

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
    //day 4 changes,for username
    map<int,string> usernames;

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
                accept(server_fd,nullptr,nullptr);

            clients.push_back(new_client);
           //new changes,for day5_username
           char name_buffer[100];
           int bytes =recv(new_client,name_buffer,sizeof(name_buffer),0);
           string username(name_buffer,bytes);

           usernames[new_client]=username;

            cout<< username<<" joined the chat"<<endl;;
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
                    cout << "[-] "
                         << usernames[client]
                         << " disconnected\n";

                    close(client);

                    it = clients.erase(it);

                    continue;
                }
                string text(buffer, bytes);
                //changes made here for private messaging (day 6)
                if(text.substr(0,5)=="/msg "){
                    stringstream ss(text);
                    string command;
                    string targetUser;
                    string privateMessage;

                    ss>>command;
                    ss>>targetUser;

                    getline(ss,privateMessage);
                    int targetSocket=findClientByUsername(usernames,targetUser);
                    if(targetSocket!=-1){
                        string pm="[PM] "+usernames[client]+": "+privateMessage;
                        send(targetSocket,pm.c_str(),pm.size(),0);
                    }

                    //cout<<"\nPrivate Message Detected\n";
                }
                else{
                    string message= usernames[client]+": "+text;
                    cout<<message<<endl;
                    for(int other:clients){
                        if(other!=client){
                            send(other,message.c_str(),message.size(),0);
                        }
                    }

                }
            }

            ++it;
        }
    }

    close(server_fd);

    return 0;
}