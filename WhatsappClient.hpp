//
// Created by Naama on 6/12/2018.
//

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
#define MAX_NAME_SIZE 30
#define MAX_MESSAGE_LEN 256
#define MAX_GROUP_SIZE 50
#define MIN_GROUP_SIZE 1
#define NUM_OF_ARGS 4
#define MAX_PORT_NUM 65535

// ------------------------------- class WhatsappClient ------------------------------- //
class WhatsappClient
{
public:
    int main(int argc, char* argv[]);
    WhatsappClient(char* clientName, char* serverAddress, char* serverPort);
    int connectToServer(char *hostname, unsigned short portnum); //tries to callSocket to the server.
//    int parseMsg(std::string msg); //parses the message and calls relevant methods for execution.
//    int validateMsg(std::string msg); // makes sure the input is valid before it will be sent to the server.
//    int validateMsg(const command_type& commandT, const std::string& name, const std::string& message,
//                    const std::vector<std::string>& clients); // makes sure the input is valid before it
    // will be sent to the server.
    int readFromServer(); //needed?
    int writeToServer(std::string msg); //needed? (writea according to the protocol)
    int getSocketHandle();
    const std::string getClientName();
//    const int isGroupMember(std::string& groupName);
    int isReceiverNotMe(std::string& receiver);
    int validateName(const std::string& name);
    int validateGroup(const std::string& name, const std::vector<std::string>& clients);
    int validateSend(std::string& receiver);
    int setLastCommand(command_type command);
    command_type getLastCommand();
    int setLastName(std::string& name);
    int inputFromUser(std::string msg);
    int inputFromServer(std::string msg);
//    int clientOutput();
    int clientOutput(command_type commandT, std::string name, std::string& messsage,
                     std::vector<std::string> clients,
                     bool success);
    bool isLastInnerMsg;
    bool initalized;
private:
//    char myName[MAX_NAME_SIZE+1];
    std::string myName;
    struct sockaddr_in sa;
    struct hostent *hp;
    int socketHandle;
    std::vector<std::string> groups = {};
    command_type lastCommand;
    std::string lastName;
    std::vector<std::string> lastClients;
    bool successFromServer;
//    char* readBuffer;
//    char* writeBuffer;

};

#endif //OS_EX4_WHATSAPPCLIENT_HPP
