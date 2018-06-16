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
#include <bits/unordered_set.h>
#include <set>
#include <regex>

// ------------------------------- constants ------------------------------- //
#define MAX_NAME_SIZE 30
#define MAX_MESSAGE_LEN 256
#define MAX_GROUP_SIZE 50
#define MIN_GROUP_SIZE 2
#define NUM_OF_ARGS 4
#define MAX_PORT_NUM 65535

// ------------------------------- class WhatsappClient ------------------------------- //
class WhatsappClient
{
public:
    int main(int argc, char* argv[]);
    WhatsappClient(char* clientName, char* serverAddress, char* serverPort);
    int connectToServer(char *hostname, unsigned short portnum); //tries to callSocket to the server.
    int parseMsg(std::string msg); //parses the message and calls relevant methods for execution.
//    int validateMsg(std::string msg); // makes sure the input is valid before it will be sent to the server.
//    int validateMsg(const command_type& commandT, const std::string& name, const std::string& message,
//                    const std::vector<std::string>& clients); // makes sure the input is valid before it
    // will be sent to the server.
    int readFromServer(); //needed?
    int writeToServer(std::string msg); //needed? (writea according to the protocol)
    int getSocketHandle();
private:
    char myName[MAX_NAME_SIZE+1];
    struct sockaddr_in sa;
    struct hostent *hp;
    int socketHandle;
    std::unordered_set<std::string> groups; // do we need map?
    char* readBuffer;
    char* writeBuffer;

};

#endif //OS_EX4_WHATSAPPCLIENT_HPP
