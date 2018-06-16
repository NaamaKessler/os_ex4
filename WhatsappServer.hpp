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
#include <errno.h>

// ------------------------------- constants ------------------------------- //
#define MAX_NAME_SIZE = 30
#define MAX_MSG_LEN = 256
#define MAX_CLIENTS_NUM


// ------------------------------- class WhatsappServer ------------------------------- //
class WhatsappServer
{
public:
    explicit WhatsappServer(unsigned short portNum);
    fd_set clientsFds;
    int genSocket;
    int establisConnection();
    int readClient(std::string clientName);
    std::map<std::string, int> getClients();

    std::map<std::string, int> connectedClients;

private:
    char myName[31];
    struct sockaddr_in sa;
    struct hostent* hp;
    std::map<std::string, std::set<int>> groups;
    int writeClient(std::string& originName, std::string& destName, std::string& message);
    int whosConnected();
    void* createGroup(std::string& clientName, std::string& nameOfGroup, std::vector<std::string>& members);
    int exitClient(std::string& name);
};

#endif //OS_EX4_WHATSAPPSERVER_HPP
