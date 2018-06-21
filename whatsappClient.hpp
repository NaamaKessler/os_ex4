#ifndef OS_EX4_WHATSAPPCLIENT_HPP
#define OS_EX4_WHATSAPPCLIENT_HPP

// ------------------------------- includes ------------------------------- //
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include "whatsappio.h"
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <set>
#include <regex>
#include <boost/algorithm/string.hpp>

// ------------------------------- constants ------------------------------- //
#define MAX_MESSAGE_LEN 256
#define MIN_GROUP_SIZE 1
#define NUM_OF_ARGS 4
#define MAX_PORT_NUM 65535
#define MIN_PORT_NUM 1024

// ------------------------------- class whatsappClient ------------------------------- //
class whatsappClient
{
public:
    /**
     * WhatsappClient constructor.
     */
    whatsappClient(char* clientName, char* serverAddress, char* serverPort);

    /**
     * WhatsappClient destructor.
     */
    ~whatsappClient();

    /**
     * tries to callSocket to the server.
     * @param hostname
     * @param portnum
     * @return 0 for success, -1 for failure.
     */
    int connectToServer(char *hostname, unsigned short portnum);

    /**
     * Reads the input from the server.
     * @return 0 for success, -1 for failure.
     */
    int readFromServer();

    /**
     * Writes to the server.
     * @param msg
     * @return totalBytesWritten
     */
    int writeToServer(std::string msg);

    /**
     * @return socket fd.
     */
    int getSocketHandle();

    /**
     * @return the client's name.
     */
    const std::string getClientName();

    /**
     * Checks if the receiver is not the sender.
     * @param receiver
     * @return 0 for success, -1 for failure.
     */
    int isReceiverNotMe(std::string& receiver);

    /**
    * validates name.
    * @param name
    * @return 0 for success, -1 for failure.
    */
    int validateName(const std::string& name);

    /**
     * validates group.
     * @param name
     * @param clients
     * @return 0 for success, -1 for failure.
     */
    int validateGroup(const std::string& name, const std::vector<std::string>& clients);

    /**
     * validates send command.
     * @param receiver
     * @param message
     * @return 0 for success, -1 for failure.
     */
    int validateSend(std::string& receiver, std::string& message);

    /**
     * Sets last command's type.
     */
    int setLastCommand(command_type command);

    /**
     * Sets lastname.
     */
    int setLastName(std::string& name);

    /**
     * Gets input from user and parses it.
     * @param msg
     * @return 0 for success, -1 for failure.
     */
    int inputFromUser(std::string msg);

    /**
     * Gets input from the server and parses it, and prints to the screen.
     * @param msg
     * @return 0 for success, -1 for failure.
     */
    int inputFromServer(std::string msg);

    /**
     * Prints the relevant message to the screen.
     * @param commandT
     * @param name
     * @param message
     * @param clients
     * @param success
     * @return 0 for success, -1 for failure.
     */
    int clientOutput(command_type commandT, std::string name, std::string& message,
                     std::vector<std::string> clients,
                     bool success);
    bool initalized;
private:
    std::string myName;
    struct sockaddr_in sa;
    struct hostent *hp;
    int socketHandle;
    command_type lastCommand;
    std::string lastName;
    std::vector<std::string> lastClients;
    bool successFromServer;

};

#endif //OS_EX4_WHATSAPPCLIENT_HPP
