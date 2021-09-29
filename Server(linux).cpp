// Includes

#include <iostream>
#include <sys/types.h> //https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/types.h.html
#include <sys/socket.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <arpa/inet.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
#include <netdb.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/netdb.h.html
#include <unistd.h> //https://pubs.opengroup.org/onlinepubs/7908799/xsh/unistd.h.html
#include <string>
#include <cstring> //https://www.cplusplus.com/reference/cstring/
#include <netinet/in.h> //https://pubs.opengroup.org/onlinepubs/009695399/basedefs/netinet/in.h.html
#include <vector> //https://www.cplusplus.com/reference/vector/vector/vector/
#include <pthread.h> //https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html
#include <set> //https://www.cplusplus.com/reference/set/set/set/

#define BACKLOG 127

using namespace std;

// Declareing Variables

int k;

// structing arguments for clients
struct args {
    int *k;
    int *clients;
    string *clientNames;
    int src;
};

// const char

char *serverName;
char *port;

// creating a set for clients
set<int> excludedList;

// Function to Send message to client

void sendToClients (int *k, int *clients, string *clientNames, int src, const char *msg, int flag = 0) {
    if (excludedList.count(src)) 
    return;

    for (int i = 0; i < *k; ++i) {
        if (excludedList.count(i)) 
        continue;
        if (i == src) 
        continue;

        // Sending messages to all clients
        string txtToSend = string(msg, 0, strlen(msg));
        if (flag == 0) 
        txtToSend = clientNames[src] + ": " + txtToSend;

        send(clients[i], txtToSend.c_str(), txtToSend.length() + 1, 0); //https://pubs.opengroup.org/onlinepubs/7908799/xns/send.html
    }
    
    //Greetings when joined
    if (flag == 1) {
        string greet = "Welcome to Server \"" + string(serverName, 0, strlen(serverName)) + "\", " + clientNames[src];
        send(clients[src], greet.c_str(), greet.length() + 1, 0);
    }
}

// Fucntion to Read message from server

void *ReadFromClient(void *p) {
    args *myArgs = ((args *) p);
    int *k = myArgs->k;
    int *clients = myArgs->clients;
    string *clientNames = myArgs->clientNames;
    int src = myArgs->src;

    if (excludedList.count(src) == 0) {
        char buff[256]{0};

        while (1) {
            memset(buff, 0, sizeof buff);
            int rbSize = recv(clients[src], buff, sizeof buff, 0);

            // if any client got disconneted
            if (rbSize <= 0) {
                string prompt = "\"" + clientNames[src] + "\" disconnected from the server";
                cout << prompt << "\n";
                sendToClients(k, clients, clientNames, src, prompt.c_str(), 2);
                excludedList.insert(src);
                close(clients[src]);
                break;
            }
            sendToClients(k, clients, clientNames, src, buff);
        }
    }
    pthread_exit(0);
}

// Main 

int main(int argc, char **argv) {

    // const arg
    serverName = argv[1];
    port = argv[2];

    // creating addrinfo (values)
    addrinfo hints, *res;

    memset(&hints, 0, sizeof hints); //memset() is used to fill a block of memory with a particular value.
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // protocol-independent translation from an ANSI host name to an address.
    getaddrinfo(NULL, port, &hints, &res); //https://linux.die.net/man/3/getaddrinfo

    // Creating Socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //https://pubs.opengroup.org/onlinepubs/7908799/xns/socket.html
    if (sockfd == -1)
        cout << "Error creating socket." << endl;

    // Binding Socket
    int bindsock = bind(sockfd, res->ai_addr, res->ai_addrlen); //https://pubs.opengroup.org/onlinepubs/7908799/xns/bind.html
    if (bindsock == 0)
        cout << "Socket successfully bindined." << endl;
    else if (bindsock == 1)
        cout << "Error binding socket." << endl;

    // Listening 
    listen(sockfd, BACKLOG); // https://pubs.opengroup.org/onlinepubs/7908799/xns/listen.html

    int newfds[BACKLOG]{0};
    sockaddr_in their_addresses[BACKLOG];
    socklen_t their_sizes[BACKLOG];

    // creating threads
    pthread_t tids_recv[BACKLOG];
    pthread_attr_t attrs_recv[BACKLOG];
    
    string clientNames[BACKLOG];

    for (int i = 0; i < BACKLOG; ++i) {
        pthread_attr_init(&attrs_recv[i]);
    }

    for (int i = 0; i < BACKLOG; ++i) {
        their_sizes[i] = sizeof their_addresses[i];

        // accepting connection from client
        newfds[i] = accept(sockfd, (sockaddr *) &their_addresses[i], &their_sizes[i]); //https://pubs.opengroup.org/onlinepubs/7908799/xns/accept.html
        
        // first request
        char tBuff[256]{0};

        // recving from client
        int rbSizeF = recv(newfds[i], tBuff, sizeof tBuff, 0); //https://pubs.opengroup.org/onlinepubs/7908799/xns/recv.html
        clientNames[i] = string(tBuff, 0, rbSizeF);
        
        // getting IP of client
        char theirIpBuff[INET_ADDRSTRLEN]{0};
        string clientIp;
        //return a pointer to the buffer containing the text string
        inet_ntop(AF_INET, &(their_addresses[i].sin_addr.s_addr), theirIpBuff, sizeof theirIpBuff); // https://pubs.opengroup.org/onlinepubs/009604499/functions/inet_ntop.html
        clientIp = string(theirIpBuff, 0, strlen(theirIpBuff));
        
        // port number of client 
        int theirPort = ntohs(their_addresses[i].sin_port); //convert values between host and network byte order

        // message to server
        string prompt = "\"" + clientNames[i] + "\" connected to the server from IP " + clientIp + ":" + to_string(theirPort);
        
        // message to other clients
        string toclientgreet = "\"" + clientNames[i] + "\" joined the chat room";
        cout << prompt << "\n";
        k++;


        sendToClients(&k, newfds, clientNames, i, toclientgreet.c_str(), 1);
        args myArgs;
        myArgs.clientNames = clientNames;
        myArgs.clients = newfds;
        myArgs.k = &k;
        myArgs.src = i;

         // creating thread object https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_create.html
        pthread_create(&tids_recv[i], &attrs_recv[i], ReadFromClient, &myArgs); 
    }

    // closing socket
    close(sockfd);

    // joining thread for all clients
    for (int i = 0; i < k; ++i) {
        pthread_join(tids_recv[i], NULL);
    }

    // closing connection when client leaves
    for (int i = 0; i < k; ++i) {
        close(newfds[i]);
    }

    return 0;
}