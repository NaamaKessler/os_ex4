//
// Created by Naama on 6/12/2018.
//

#include "WhatsappServer.hpp"

/**
 * initializes the server and opens a general requests socket.
 */
WhatsappServer::WhatsappServer()
{

}

/**
 * Accepts a connection request and opens a socket for communication with the client.
 * @return
 */
int WhatsappServer::acceptConnection()
{
    // initialize protocol interpreter
//    this->commandInterpreter->insert(std::pair<std::string, void* (*)(void*)>())

    // init hostent
    if (gethostname(this->myName, 30) == -1) //todo - macro raises error
    {
        //error
        return 1;
    }
    this->hp = gethostbyname(this->myName);
    if (!this->hp)
    {
        //error
    }

    // init sockets address
    memset(&this->sa, 0, sizeof(struct sockadder_in));
    sa.sin_family = hp->h_addrtype;
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_port = htons(NULL); // todo - how to determine portnum?

    // init and bind listening socket
    if ((this->genSocket = socket(AF_INET, SOCK_STREAM, 0) < 0))
    {
        //error
    }
    if (bind(this->genSocket, (struct sockaddr*)&this->sa, sizeof (struct sockaddr_in)) < 0)
    {
        //error
        close(this->genSocket);
    }

//    listen(this->genSocket, ); //todo - ????
    fd_set selectSet;
    while (true) //todo - should be in here or in the main?
    {
        selectSet = this->clientsFds; // will be changed in-place to contain only ready fds
        int ready = select(100, &selectSet, NULL, NULL, NULL); // todo - what's the max fds num?
//        for (fd_set fd: this->clientsFds)
//        {
////            if (FD_ISSET(fd, &selectSet))
////            {
////
////            }
//
//        }
    }


}

/**
 * Reads messages from the client and carries them out.
 * @return
 */
int WhatsappServer::readClient()
{

}

/**
 * Writes messages to the client.
 * @return
 */
int WhatsappServer::writeClient()
{

}// writes msg according to the protocol

/**
 * @return a list containing all connected clients names.
 */
int WhatsappServer::whosConnected()
{

}

/**
 * Executes exit request of the client.
 * @return
 */
int WhatsappServer::exitClient()
{

}

/**
 * Removes a client from a group.
 * @return
 */
int WhatsappServer::createGroup()
{

}

/**
 * initializes a socket.
 * @return fd of the new socket.
 */
int WhatsappServer::initSocket()
{

}