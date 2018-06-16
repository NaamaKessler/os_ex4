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
    if (gethostname(this->myName, 30) == -1) // macro raises error -?
    {
        print_error("gethostname", errno);
    }
    this->hp = gethostbyname(this->myName);
    if (!this->hp)
    {
        print_error("gethostbyname", errno);
    }

    // init sockets address
    memset(&this->sa, 0, sizeof(struct sockadder_in));
    sa.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&sa.sin_addr, hp->h_addr, (size_t)hp->h_length);
    sa.sin_port = htons(portNum);

    // init and bind listening socket
    if ((this->genSocket = socket(AF_INET, SOCK_STREAM, 0) < 0))
    {
        print_error("socket", errno);
    }
    if (bind(this->genSocket, (struct sockaddr*)&this->sa, sizeof (struct sockaddr_in)) < 0)
    {
        print_error("bind", errno);
        close(this->genSocket);
    }

    listen(this->sa.sin_port, 10);
}

/**
 * Accepts a connection request and opens a socket for communication with the client.
 * @return
 */
int WhatsappServer::establisConnection() //todo - how do i get the client's name?
{
    int newSockFd = accept(this->genSocket, nullptr, nullptr);
    if (newSockFd == -1)
    {
        //error
        print_fail_connection();
        return 1;
    }
    else
    {
        // todo- add to clients map in the server + get it's name
//        print_connection_server();
    }
}

/**
 * Reads messages from the client and carries them out.
 * @return
 */
int WhatsappServer::readClient(std::string clientName)
{
    int clientFd = this->connectedClients[clientName];
    // read message from buffer
    auto buf = new char[257];
    memset(buf, '\0', 257);
    int byteCount = 0;
    int byteRead = 0;
    while (byteCount < 256)
    {
        byteRead = (int)read(clientFd, buf, (size_t)256-byteCount);
        if (byteRead > 0)
        {
            byteCount += byteRead;
            buf += byteRead;
        }
        if (byteRead < 1)
        {
            print_error("read", errno);
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
            writeClient(clientName, name, messsage);
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
int WhatsappServer::writeClient(std::string& originName, std::string& destName, std::string& message)
{
    // verify that the client exists:
    if (this->connectedClients.empty() ||
            this->connectedClients.find(destName) == this->connectedClients.end())
    {
        print_send(true, false, originName, destName, message);
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
            print_error("write", errno);
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
void* WhatsappServer::createGroup(std::string& clientName, std::string& nameOfGroup,
                                  std::vector<std::string>& members)
//todo - is the client part of the list?
{
    if ((this->groups.find(nameOfGroup) == this->groups.end()) &&
            (this->connectedClients.find(nameOfGroup) == this->connectedClients.end()))
    {
        // groupName is unique
        this->groups[nameOfGroup]; //initialization of set is ok?
        for (std::string& member : members)
        {
            if ((this->connectedClients.find(member) != this->connectedClients.end())) //the client exists
            {
                this->groups[nameOfGroup].insert(this->connectedClients[member]);
                // set ensures that no member will be inserted twice.
            }
            else
            {
                //todo - invalid member name - delete group?
                std::cout << members[0] << ": ERROR: failed to create group \""
                          << nameOfGroup <<"\"." << std::endl;
                this->groups.erase(nameOfGroup);
                return nullptr;
            }
        }
        this->groups[nameOfGroup].insert(this->connectedClients[clientName]);
        std::cout << members[0] << ": Group \"" <<  nameOfGroup <<
                  " \" was created successfully." << std::endl;
    }
    else
    {
        // group name already exists
        std::cout << members[0] << ": ERROR: failed to create group \""
                  << nameOfGroup <<"\"." << std::endl;
    }
} //todo - when adding to a group, do we eant to save for each client all the groups he belongs to?

std::map<std::string, int> WhatsappServer::getClients()
{
    return this->connectedClients;
}


int main (int argc, char *argv[])
{
    if (argc != 2) {
        print_server_usage();
    }
    try {
        auto portNum = (unsigned short) std::stoi(argv[1]);
        auto server = new WhatsappServer(portNum);
        bool exit = false;
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(server->genSocket, &readFds);
        FD_SET(STDIN_FILENO, &readFds);
        while (!exit)
        {
            readFds = server->clientsFds;
            if (select(MAX_CLIENTS_NUM +1, &readFds, nullptr, nullptr, nullptr) == -1) {
                print_error("select", errno);
            }

            if (FD_ISSET(server->genSocket, &readFds)) {
                // someone tried to connect (using the listening socket)
                server->establisConnection();
            }
            if (FD_ISSET(STDIN_FILENO, &readFds)) {
                // todo - handle terminal input - exit and anything else?
                std::string inputLine;
                getline(std::cin, inputLine);
                if (strcmp(inputLine, "EXIT"))
                {
                    exit = true;
                    print_exit();
                }
            }

            for (std::pair<std::string, int>& client: server->getClients())
            {
                if (FD_ISSET(client.second, &readFds))
                {
                    server->readClient(client.first);
                }
            }
        }
    }
    catch (std::invalid_argument &e)
    {
        print_server_usage();
        exit(1); //todo - ok?
    }
}



