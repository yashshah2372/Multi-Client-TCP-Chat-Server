// Includes

#include <iostream>
#include <sys/types.h> //https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/types.h.html
#include <WS2tcpip.h> //https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-basic-winsock-application
#include <string>
#include <cstring> //https://www.cplusplus.com/reference/cstring/
#include <vector> //https://www.cplusplus.com/reference/vector/vector/vector/
#include <thread> // https://www.cplusplus.com/reference/thread/thread/

#pragma comment(lib, "ws2_32.lib") // To include ws2_32 lib

using namespace std;

// Declaring variables
int k;

string name;
string serveradd;
string portnum;

bool stop_flag = false;

// Function to read from Server
void ReadFromServer(void* p) {
    int sockfd = *((int*)p);

    char buff[256]{ 0 };
    while (1) {
        memset(buff, 0, sizeof buff);   //memset() is used to fill a block of memory with a particular value.
        
        // reciving from server 
        int rbSize = recv(sockfd, buff, sizeof buff, 0); // https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
        
        // if server is disconnected
        if (rbSize <= 0) {
            cout << "The Server Disconnected\n";
            stop_flag = true;
            break;
        }
        cout << buff << "\n";
    }
}

// Function to Send message to Server

void SendToServer(void* p) {
    int sockfd = *((int*)p);

    //if server disconneted
    while (1) {
        if (stop_flag)
        break;
        
        // user input
        string str;
        getline(cin, str); 

        // sending input to server
        send(sockfd, str.c_str(), str.length() + 1, 0); //https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
    }
}

// Main 

int main() {

    WSAData wsData; //The WSADATA structure contains information about the Windows Sockets implementation.
    WSAStartup(MAKEWORD(2, 2), &wsData);  //The WSAStartup function initiates use of the Winsock DLL by a process.

    cout<<"\n\t\t\t\t************************";
    cout<<"\n\t\t\t\t* WELCOME TO CHAT ROOM *";
    cout<<"\n\t\t\t\t************************" << endl;

    // Asking for input and converting string to const char

    cout << "Enter your username" << "\n";
    cin >> name;
    const char *clientName = name.c_str();

    cout << "Enter IP address of server" << "\n";
    cin >> serveradd;
    const char *serverIP = serveradd.c_str();

    cout << "Enter port" << "\n";    
    cin >> portnum;
    const char *port = portnum.c_str();
    
    // creating hints (valus for socket)

    addrinfo hints, * res;
    memset(&hints, 0, sizeof hints); //memset() is used to fill a block of memory with a particular value.
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
 
    // https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
    getaddrinfo(serverIP, port, &hints, &res);

    // creating sockets
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    // conneting to server
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    // sending to server for first time
    send(sockfd, clientName, strlen(clientName) + 1, 0);

    // creating thread https://www.cplusplus.com/reference/thread/thread/thread/

    thread thread1(ReadFromServer, &sockfd);
    thread thread2(SendToServer, &sockfd);

    // joining threads  https://www.cplusplus.com/reference/thread/thread/join/
    thread1.join();
    thread2.join();

    // socket close and cleanups
    closesocket(sockfd);
    WSACleanup();
    return 0;
}