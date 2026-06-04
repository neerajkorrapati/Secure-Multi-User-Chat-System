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
    //CREATE SERVER
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
//STORE THE USERS/REMEMBER THE CLIENTS
    vector<int> clients;
    //day 4 changes,for username
    map<int,string> usernames;

    cout << "Server listening on port "
         << PORT << endl;

    while(true)
    {
        fd_set readfds;
//WAIT FOR ACTIVITY
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        int maxfd = server_fd;
        for(int client : clients)
        {
            FD_SET(client, &readfds);
            if(client > maxfd)
                maxfd = client;
        }

        int activity = select(maxfd + 1,&readfds,nullptr,nullptr,nullptr);

        if(activity < 0)
            continue;
//ACCEPT NEW CLIENTS
        // New connection   
        if(FD_ISSET(server_fd, &readfds))
        {
            int new_client = accept(server_fd,nullptr,nullptr);

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
//PROCESS MESSAGES
            if(FD_ISSET(client, &readfds))
            {
                int bytes =
                    recv(client,
                         buffer,
                         sizeof(buffer),
                         0);
// DISCONNECT HANDLING
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

                //Adding HELP FUNCTION :
                
                if(text=="/help"){
                    string function_list="Available Commands: \n";
                    function_list+="/help \n";
                    function_list+="/users \n";
                    function_list+="/msg <username> <message> \n";

                    //to send this to the client
                    send(client,function_list.c_str(),function_list.size(),0);

                }

                //to display USER LIST ,PRIVATELY
   
                else if(text=="/users"){
                    string users_list="Online Users: \n";
                    for(auto& pair:usernames){
                        users_list+=pair.second + "\n";
                    }
                    send(client,users_list.c_str(),users_list.size(),0);


                }
                //changes made here for PRIVATE MESSAGING HANDLER (day 6)
                else if(text.substr(0,5)=="/msg "){
                    stringstream ss(text);
                    string command;//here command stores: /msg,
                    string targetUser;//target user stores username
                    string privateMessage;//stores the remaining, i.e the message

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
                    // BROADCAST LOGIC
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