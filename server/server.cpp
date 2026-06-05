#include <iostream>
#include <vector>
#include<map>
#include<sstream>
#include<sqlite3.h> 

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
string historyResult;

//CALLBACK FUCNTION:

int historyCallback(
    void*,
    int argc,
    char** argv,
    char**)
{
    cout << "CALLBACK RUNNING" << endl;

    historyResult +=
        string(argv[1]) +
        ": " +
        string(argv[2]) +
        "\n";

    return 0;
}

int main()
{
    sqlite3* db;
    sqlite3_open("chat.db",&db);
    const char* createTable =
    "CREATE TABLE IF NOT EXISTS messages ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "username TEXT,"
    "message TEXT"
    ");";
    sqlite3_exec(db,createTable,nullptr,nullptr,nullptr);
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
                    string leave_message=usernames[client]+" left the chat";
                    cout<<leave_message<<endl;
                    for(int other : clients){
                        if(other!=client){
                            send(other,leave_message.c_str(),leave_message.size(),0);
                        }
                    }

                    close(client);
                    usernames.erase(client);
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
                    function_list+="/history\n";

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
                //adding message history view:

                else if(text=="/history"){
                 historyResult="Recent Messages:\n";
                 string sql =
                 "SELECT * FROM messages "
                 "ORDER BY id DESC "
                 "LIMIT 10;";
        
                cout << sql << endl;
                sqlite3_exec(db,sql.c_str(),historyCallback,nullptr,nullptr);

                send(client,historyResult.c_str(),historyResult.size(),0);

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
                    //SQL INTERFACE BEIGN ADDED;
                    string sql="INSERT INTO messages"
                                "(username,message) VALUES (' "+
                                usernames[client]+
                                "','"+
                                text+
                                "');";
                                cout<<"sql"<<endl;
                    sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr);


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
    sqlite3_close(db);
    return 0;
}