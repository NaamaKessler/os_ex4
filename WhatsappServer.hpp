//
// Created by Naama on 6/12/2018.
//

#ifndef OS_EX4_WHATSAPPSERVER_HPP
#define OS_EX4_WHATSAPPSERVER_HPP

// ------------------------------- includes ------------------------------- //
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <map>

// ------------------------------- constants ------------------------------- //
#define MAX_NAME_SIZE = 30


// ------------------------------- class WhatsappServer ------------------------------- //
class WhatsappServer
{
public:
    WhatsappServer(); //(initializes class & a listening socket and calls bind() and listen()?)
private:
//    char* myName; //?
    char myName[31];
    struct sockaddr_in sa;

    struct hostent* hp;
    int genSocket; // fd of the general listening socket.
    std::map<std::string, int> connectedClients; //int - Fds of the sockets.
    fd_set clientsFds;
    std::map<std::string, int> groups;
    std::map<std::string, void* (*)(void*)> commandInterpreter; //links protocol to actual functions
    int initSocket(); //creates a new socket
    int acceptConnection(); //accepts a client request and opens a socket for their communication.
    int readClient(); // reads msg and parses it according to the protocol
    int writeClient(); // writes msg according to the protocol
    int whosConnected(); //returns client names.
    int createGroup();
    int exitClient();
};

#endif //OS_EX4_WHATSAPPSERVER_HPP
