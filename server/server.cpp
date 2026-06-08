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
    //FUNCTION FOR USER REGISTRATION:
   //USING SQLITE3
   void registerUser(int client,sqlite3* db,const string &username,const string &password){
    string sql=
    "INSERT INTO users "
    "(username,password) VALUES ('"+
    username+
    "','"+
    password+
    "');";
    int rc=sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr);
    string response;

    if(rc == SQLITE_OK){
        response="Registration Succesfull";
    }
    else{
        response="Username already exists";
    }
    send(client,response.c_str(),response.size(),0);
   }

//LOGIN USERS

   bool loginUser(
    sqlite3* db,
    const string& username,
    const string& password)
{
    string sql =
        "SELECT * FROM users "
        "WHERE username='" +
        username +
        "' AND password='" +
        password +
        "';";

    bool found = false;

    auto callback =
    [](void* data,
       int,
       char**,
       char**)
    {
        bool* result =
            (bool*)data;

        *result = true;

        return 0;
    };

    sqlite3_exec(
        db,
        sql.c_str(),
        callback,
        &found,
        nullptr);

    return found;
}

    string historyResult;
    //CALLBACK FUCNTION:
    int historyCallback(
        void*,
        int argc,
        char** argv,
        char**)
    {
       // cout << "CALLBACK RUNNING" << endl;

        historyResult +=
            string(argv[1]) +
            ": " +
            string(argv[2]) +
            "\n";

        return 0;
    }

    //day 11 adding modularity to code , for reusability;

    void saveMessage(sqlite3* db,const string& username,const string& text){
        string sql=
        "INSERT INTO messages "
        "(username,message) VALUES ('" +
        username+
        "','"+
        text+
        "');";
        
        sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr);  
    }



    void broadcastMessage(const vector<int>& clients,int sender, const string& message)
    {
        for(int other:clients){
            if(other!=sender){
                send(other,message.c_str(),message.size(),0);
            }
        }
    }


    void showUsers(int client,const map<int,string>& usernames){
        string userlist="Online Users: \n";
        for(auto& pair:usernames){
            userlist+=pair.second +"\n";
        }
        send(client,userlist.c_str(),userlist.size(),0);
    }

    //show help functions:
    void showHelp(int client)
    {
        string helpMessage =
            "Available Commands:\n"
            "/help\n"
            "/users\n"
            "/msg <user> <message>\n"
            "/history\n"
            "/register <username> <password>\n"
            "/login <username> <password>\n";

        send(client,helpMessage.c_str(),helpMessage.size(),0);
    }

    //show_history_function
    void showHistory(int client,sqlite3* db){
        historyResult="Recent Messages:\n";

        string sql=
        "SELECT * FROM messages "
        "ORDER BY id DESC "
        "LIMIT 10;";

        sqlite3_exec(db,sql.c_str(),historyCallback,nullptr,nullptr);
        send(client,historyResult.c_str(),historyResult.size(),0);
    }


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
        sqlite3* db;
        sqlite3_open("chat.db",&db);
        const char* createTable =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT,"
        "message TEXT"
        ");";
        sqlite3_exec(db,createTable,nullptr,nullptr,nullptr);

        //password storage sql:

        const char* createUsersTable =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "password TEXT"
        ");";

        sqlite3_exec(db,createUsersTable,nullptr,nullptr,nullptr); //willstore : id || username || password

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
                int new_client =
                accept(server_fd,nullptr,nullptr);

                clients.push_back(new_client);

                usernames[new_client] = "Guest";

        cout << "New client connected"
     << endl;
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
                        showHelp(client);

                    }

                    //to display USER LIST ,PRIVATELY
    
                    else if(text=="/users"){
                        showUsers(client,usernames);
                    }
                    //adding message history view:

                    else if(text=="/history"){
                    showHistory(client,db);
                    }
                    else if(text.substr(0,10)=="/register "){
                        stringstream ss(text);

                        string command;
                        string username;
                        string password;

                        ss>>command;
                        ss>>username;
                        ss>>password;
                        
                        registerUser(client,db,username,password);


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
                    else if(text.substr(0,7)=="/login "){
                        stringstream ss(text);
                        string command;
                        string username;
                        string password;

                        ss>>command;
                        ss>>username;
                        ss>>password;
                        bool success= loginUser(db,username,password);
                        string response;
                        if(success){
                            usernames[client]=username;
                            response="Login Successful. Welcome "+username;
                            //response="Login Successful";
                        }
                        else{
                            response="Invalid Username or Password";
                        }
                        send(client,response.c_str(),response.size(),0);
                    }
                    else{
                        // BROADCAST LOGIC
                        string message= usernames[client]+": "+text;
                        //SQL INTERFACE BEIGN ADDED;
                        saveMessage(db,usernames[client],text);


                        cout<<message<<endl;
                        broadcastMessage(clients,client,message);

                    }
                }

                ++it;
            }
        }

        close(server_fd);
        sqlite3_close(db);
        return 0;
    }