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
    int listeningSocket;

    /**
    * initializes the server and opens a general requests socket.
    */
    explicit WhatsappServer(unsigned short portNum);

    /**
    * Accepts a connection request and opens a socket for communication with the client.
    */
    int establishConnection();

    /**
    * Reads messages from the client and carries them out.
    * @return
    */
    int readClient(std::string clientName);

    /**
    * Return the map of clients and their Fds.
    * @return
    */
    const std::map<std::string, int> getClients() const;

private:
    char myName[31];
    struct sockaddr_in sa;
    struct hostent* hp;
    std::map<std::string, int> connectedClients;
    std::map<std::string, std::set<int>> groups;

    /**
    * Creates a group of clients.
    * @return
    */
    int createGroup(std::string& clientName, std::string& groupName,
                      std::vector<std::string>& members);

    /**
    * Sends message from one client to the another.
    * @return
    */
    int sendMessage(std::string &originName, std::string &destName, std::string &message);

    /**
    * @return a list containing all connected clients names.
    */
    int whosConnected();

    /**
    * Executes exit request of the client.
    * @return
    */
    int exitClient(std::string& name);
};

#endif //OS_EX4_WHATSAPPSERVER_HPP
