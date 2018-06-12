//
// Created by Naama on 6/12/2018.
//

#ifndef OS_EX4_WHATSAPPSERVER_HPP
#define OS_EX4_WHATSAPPSERVER_HPP

// ------------------------------- includes ------------------------------- //
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_NAME_SIZE = 30

// ------------------------------- struct hostnet ------------------------------- //
struct hostnet
{
    char* h_name;
    int h_adddrtype;
    int h_length;
    char* h_addr;
};



// ------------------------------- class WhatsappServer ------------------------------- //
class WhatsappServer
{
public:
    WhatsappServer(); //(initializes class & a listening socket and calls bind() and listen()?)
    int acceptConnection(); //accepts a client request and opens a socket for their communication.
    int readClient(); // reads msg and parses it according to the protocol
    int writeClient(); // writes msg according to the protocol
private:
    // add - array of connected client sockets ?

    char* myName;
    struct sockaddr_in sa;
    struct hostnet* hp;

    int initSocket(); //creates a new socket
};

#endif //OS_EX4_WHATSAPPSERVER_HPP
