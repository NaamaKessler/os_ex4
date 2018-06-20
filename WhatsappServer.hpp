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
#include <algorithm>

// ------------------------------- constants ------------------------------- //

#define DEFAULT_CLIENT_NAME "???" // place-holder for a new client's name


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
     * Destructor.
     */
    ~WhatsappServer();

    /**
    * Accepts a connection request and opens a socket for communication with the client.
    * @return 1 on success, 0 otherwise.
    */
    int establishConnection();

    /**
    * Reads messages from the client and carries them out.
    * @return 1 upon success, 0 upon failure.
    */
    void readClient(std::string clientName, int clientFd);

    /**
    * Return the map of clients and their Fds.
    * @return 1 upon success, 0 upon failure.
    */
    std::map<std::string, int> getClients();

    /**
    * Signals to client that the server has crashed / got an EXIT command.
    */
    void signalExit(const std::string& clientName);

private:
    char myName[31];
    struct sockaddr_in sa;
    struct hostent* hp;
    std::map<std::string, int> connectedClients;
    std::map<std::string, std::set<std::string>> groups;

    /**
    * Creates a group of clients.
    * @return 1 on success, 0 otherwise.
    */
    int createGroup(std::string& clientName, std::string& groupName,
                      std::vector<std::string>& members);

    /**
    * Sends message from one client to the another.
    * @return 1 on success, 0 otherwise.
    */
    int sendMessage(command_type command, std::string &originName, int origFd, const std::string &destName,
                    std::string &message);

    /**
     * Sends a message to all groupName members.
     * @return 1 upon success, 0 upon failure.
     */
    int sendToGroup(std::string& groupName, std::string& origName, std::string& msg);

    /**
    * Lists all connected clients names.
     * @return 1 on success, 0 otherwise.
    */
    int whosConnected(std::string& clientName);

    /**
    * Executes exit request of the client.
    * @return 1 on success, 0 otherwise.
    */
    int exitClient(std::string& name);

    /**
     * Receives a new client's name and inserts it in the appropriate pair of this->ConnectedClients
     * (which currently stores a place-saver).
     * @return 1 on success, 0 otherwise.
     */
    int insertName(int fd, std::string& name);

    /**
     * Signals the client that the request has succeeded/failed.
     * @param clientFd
     * @param success
     */
    void echoClient(command_type command, std::string& clientName, int clientFd, int success);


};

#endif //OS_EX4_WHATSAPPSERVER_HPP
