// Includes

#include <iostream>
#include <sys/types.h> //https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/types.h.html
#include <sys/socket.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <arpa/inet.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
#include <netdb.h> //https://pubs.opengroup.org/onlinepubs/7908799/xns/netdb.h.html
#include <unistd.h> //https://pubs.opengroup.org/onlinepubs/7908799/xsh/unistd.h.htm
#include <string>
#include <cstring> //https://www.cplusplus.com/reference/cstring/
#include <netinet/in.h> //https://pubs.opengroup.org/onlinepubs/009695399/basedefs/netinet/in.h.html
#include <pthread.h> //https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html

using namespace std;

// Declareing variables
int k;

string name;
string serveradd;
string portnum;

bool stop_flag = false;

// Function to Read From Server

void *ReadFromServer(void *p) {
    int sockfd = *((int *) p);
    // buff messages

    char buff[256]{0};
    while (1) {
        memset(buff, 0, sizeof buff); //memset() is used to fill a block of memory with a particular value.

        // reciving from server
        int rbSize = recv(sockfd, buff, sizeof buff, 0); // https://pubs.opengroup.org/onlinepubs/7908799/xns/recv.html

        // if server disconneted
        if (rbSize <= 0) {
            cout << "The Server Disconnected\n";
            stop_flag = true;
            break;
        }
        // print out message
        cout << buff << "\n";
    }
    pthread_exit(0);
}

// Function to Send message to Server

void *SendToServer(void *p) {
    int sockfd = *((int *) p);

    // stop sending when server is disconnected
    while (1) {
        if (stop_flag) break;

        // user input
        string str;
        getline(cin, str);

        //sending input to server
        send(sockfd, str.c_str(), str.length() + 1, 0); //https://pubs.opengroup.org/onlinepubs/7908799/xns/send.html
    }
    pthread_exit(0);
}

// Main Program

int main() {

    cout << "\n\t\t\t\t************************";
    cout << "\n\t\t\t\t* WELCOME TO CHAT ROOM *";
    cout << "\n\t\t\t\t************************" << endl;

    // Asking for input and converting string to const char

    cout << "Enter your username" << "\n";
    cin >> name;
    const char *clientName = name.c_str();

    cout << "Enter your serverip" << "\n";
    cin >> serveradd;
    const char *serverIp = serveradd.c_str();

    cout << "Enter your port" << "\n";    
    cin >> portnum;
    const char *port = portnum.c_str();

    // creating hints (valus for socket)
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints); //memset() is used to fill a block of memory with a particular value.
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // protocol-independent translation from an ANSI host name to an address.
    getaddrinfo(serverIp, port, &hints, &res); //https://man7.org/linux/man-pages/man3/getaddrinfo.3.html

    // createing socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
    
    // connecting socket to server
    connect(sockfd, res->ai_addr, res->ai_addrlen); //https://pubs.opengroup.org/onlinepubs/7908799/xns/connect.html

    // sendind detils to server for first time
    send(sockfd, clientName, strlen(clientName) + 1, 0);

    // constructing thread 

    pthread_t tid_recv, tid_send;
    pthread_attr_t attr_recv, attr_send;

    // initialising threads attribute object https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_attr_init.html
    
    pthread_attr_init(&attr_recv);
    pthread_attr_init(&attr_send);

    // creating thread object https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_create.html

    pthread_create(&tid_recv, &attr_recv, ReadFromServer, &sockfd);
    pthread_create(&tid_send, &attr_send, SendToServer, &sockfd);

    // joining thread object https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_join.html
    
    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(sockfd);

    return 0;
}