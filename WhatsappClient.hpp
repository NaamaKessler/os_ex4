//
// Created by Naama on 6/12/2018.
//

#ifndef OS_EX4_WHATSAPPCLIENT_HPP
#define OS_EX4_WHATSAPPCLIENT_HPP

// ------------------------------- includes ------------------------------- //
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "whatsappio.h"

// ------------------------------- constants ------------------------------- //
#define MAX_NAME_SIZE = 30
#define MAX_MESSAGE_LEN = 256
#define MAX_GROUP_SIZE = 50

// ------------------------------- class WhatsappClient ------------------------------- //
class WhatsappClient
{
public:
    WhatsappClient();
    int connect(char* hostname, unsigned short portnum); //tries to connect to the server.
    int parseMsg(std::string msg); //parses the message and calls relevant methods for execution.
    int validateMsg(std::string msg); // makes sure the input is valid before it will be sent to the server.
    int readFromServer(); //needed?
    int writeToServer(std::string msg); //needed? (writea according to the protocol)
private:
};

#endif //OS_EX4_WHATSAPPCLIENT_HPP
