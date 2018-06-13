//
// Created by Naama on 6/12/2018.
//

#include "WhatsappServer.hpp"

/**
 * initializes the server and opens a general requests socket.
 */
WhatsappServer::WhatsappServer(unsigned short portNum)
{
    // init hostent
    if (gethostname(this->myName, 30) == -1) // macro raises error
    {
        //error
    }
    this->hp = gethostbyname(this->myName);
    if (!this->hp)
    {
        //error
    }

    // init sockets address
    memset(&this->sa, 0, sizeof(struct sockadder_in));
    sa.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&sa.sin_addr, hp->h_addr, (size_t)hp->h_length);
    sa.sin_port = htons(portNum);

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
}

/**
 * Accepts a connection request and opens a socket for communication with the client.
 * @return
 */
int WhatsappServer::acceptConnection()
{

}

/**
 * Reads messages from the client and carries them out.
 * @return
 */
int WhatsappServer::readClient(int fd) // fd or name? who calls it?
{
    // read message from buffer
    auto buf = new char[257];
    memset(buf, '\0', 257);
    int byteCount = 0;
    int byteRead = 0;

    while (byteCount < 256)
    {
        byteRead = (int)read(fd, buf, (size_t)256-byteCount);
        if (byteRead > 0)
        {
            byteCount += byteRead;
            buf += byteRead;
        }
        if (byteRead < 1)
        {
            //error
        }
    }

    // parse command:
    command_type commandT;
    std::string name;
    std::string messsage;
    std::vector<std::string> clients;
    parse_command(buf, commandT, name, messsage, clients);

    switch (commandT)
    {
        case CREATE_GROUP:
            createGroup(name, clients); //todo - does 'clients' includes the sender's name?
            break;
        case SEND:
            writeClient(name, messsage);
            break;
        case WHO:
            whosConnected();
            break;
        case EXIT:
            exitClient(name);
            break;
        default: //we won't get here since Client makes sure that the command type is valid.
            break;
    }
    delete buf;




}

/**
 * Writes message to the client.
 * @return
 */
int WhatsappServer::writeClient(std::string& destName, std::string& message)
{
    // verify that the client exists:
    if (this->connectedClients.find(destName) == this->connectedClients.end()) //what happens if map is empty?
    {
        //error
    }

    int byteCount = 0;
    int byteWritten = 0;
    while (byteCount < 256)
    {
        byteWritten = (int)write(this->connectedClients.at(destName),
                                 message.c_str() + byteWritten, (size_t)256-byteCount); // ok?
        if (byteWritten > 0)
        {
            byteCount += byteWritten;
        }
        else
        {
            //error
        }
    }
}

/**
 * @return a list containing all connected clients names.
 */
int WhatsappServer::whosConnected()
{
    auto clientsNames = new char[(30*this->connectedClients.size()) + 1];
    memset(clientsNames, '\0', (30*this->connectedClients.size()) + 1);
    for (const auto& pair: this->connectedClients)
    {
        memcpy(clientsNames, pair.first, sizeof(char)*pair.first.length());
    }
    print_who_server(clientsNames);
    delete clientsNames;
}

/**
 * Executes exit request of the client.
 * @return
 */
int WhatsappServer::exitClient(std::string& name)
{

}

/**
 * Removes a client from a group.
 * @return
 */
void* WhatsappServer::createGroup(std::string& nameOfGroup, std::vector<std::string>& members)
{

}

/**
 * initializes a socket.
 * @return fd of the new socket.
 */
int WhatsappServer::initSocket()
{

}