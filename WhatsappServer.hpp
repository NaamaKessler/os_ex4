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
#include "whatsappio.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <set>

// ------------------------------- constants ------------------------------- //
#define MAX_NAME_SIZE = 30
#define MAX_MSG_LEN = 256
#define MAX_CLIENTS_NUM


// ------------------------------- class WhatsappServer ------------------------------- //
class WhatsappServer
{
public:
    explicit WhatsappServer(unsigned short portNum); //(initializes class & a listening socket and calls bind() and listen()?)
    fd_set clientsFds; //add clients here uppon connectino
    int genSocket; // fd of the general listening socket.
    int establisConnection(); //accepts a client request and opens a socket for their communication.
    int readClient(std::string clientName); // reads msg and parses it according to the protocol
    int getConnenctionsNum();
    std::map<std::string, int> getClients();

    std::map<std::string, int> connectedClients; //int - Fds of the sockets.

private:
    char myName[31];
    struct sockaddr_in sa;
    struct hostent* hp;
    std::map<std::string, std::set<int>> groups;
    std::map<std::string, void* (*)(void*)> commandInterpreter; //links protocol to actual functions
    int initSocket(); //creates a new socket
    int writeClient(std::string& destName, std::string& messsage); // writes msg to client according to the protocol
    int whosConnected(); //returns client names.
    void* createGroup(std::string& nameOfGroup, std::vector<std::string>& members);
    int exitClient(std::string& name);
};

#endif //OS_EX4_WHATSAPPSERVER_HPP
