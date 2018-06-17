//
// Created by Naama on 6/12/2018.
//

// ------------------------------- includes ------------------------------- //
#include "WhatsappServer.hpp"


// ------------------------------- public methods ------------------------------- //
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
    if ((this->listeningSocket = socket(AF_INET, SOCK_STREAM, 0) < 0))
    {
        print_error("socket", errno);
    }
    if (bind(this->listeningSocket, (struct sockaddr*)&this->sa, sizeof (struct sockaddr_in)) < 0)
    {
        print_error("bind", errno);
        close(this->listeningSocket);
    }

    if (listen(this->sa.sin_port, 10) < 0)
    {
        print_error("listen", errno);
    }
}

/**
 * Accepts a connection request and opens a socket for communication with the client.
 */
int WhatsappServer::establishConnection() //todo - how do i get the client's name?
{
    int newSockFd = accept(this->listeningSocket, nullptr, nullptr);
    if (newSockFd < 0)
    {
        print_fail_connection();
        return 1;
    }
    else
    {
        // todo- add to clients map in the server + get it's name
//        print_connection_server();
        return 0;
    }
}

/**
 * Reads messages from the client and carries them out.
 * @return
 */
int WhatsappServer::readClient(std::string clientName)
{
    int clientFd = this->connectedClients[clientName];
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
            return 1;
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
            createGroup(clientName, name, clients);
            break;
        case SEND:
            sendMessage(clientName, name, messsage);
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
    return 0;
}

/**
 * Returns the map of clients and their Fds.
 * @return
 */
const std::map<std::string, int> WhatsappServer::getClients() const
{
    return this->connectedClients;
}

/**
 * Creates a group of clients.
 * @return
 */
int WhatsappServer::createGroup(std::string& clientName, std::string& groupName,
                                  std::vector<std::string>& members)
{
    if ((this->groups.find(groupName) == this->groups.end()) &&
        (this->connectedClients.find(groupName) == this->connectedClients.end()))
    {
        // groupName is unique
        this->groups[groupName]; //initialization of set is ok?
        for (std::string& member : members)
        {
            if ((this->connectedClients.find(member) != this->connectedClients.end())) //the client exists
            {
                this->groups[groupName].insert(this->connectedClients[member]);
                // set ensures that no member will be inserted twice.
            }
            else
            {
                print_create_group(true, false, clientName, groupName);
                this->groups.erase(groupName);
                return 1;
            }
        }
        this->groups[groupName].insert(this->connectedClients[clientName]); //add creator to group
        print_create_group(true, true, clientName, groupName);
        return 0;
    }
    else  // group name already exists
    {
        print_create_group(true, false, clientName, groupName);
        return 1;
    }
}


// ------------------------------- private methods ------------------------------- //
/**
 * Sends message fron one client to the another.
 * @return
 */
int WhatsappServer::sendMessage(std::string &originName, std::string &destName,
                                std::string &message)
{
    // verify that the client exists:
    if (this->connectedClients.empty() ||
            this->connectedClients.find(destName) == this->connectedClients.end())
    {
        print_send(true, false, originName, destName, message);
        return 1;
    }
    int byteCount = 0;
    int byteWritten = 0;
    while (byteCount < 256)
    {
        byteWritten = (int)write(this->connectedClients.at(destName),
                                 message.c_str() + byteWritten, (size_t)256-byteCount);
        if (byteWritten > 0)
        {
            byteCount += byteWritten;
        }
        else
        {
            print_error("write", errno);
            return 1;
        }
    }
    print_message(originName, message);
    return 0;
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
    return 0;
}

/**
 * Executes exit request of the client.
 * @return
 */
int WhatsappServer::exitClient(std::string& clientName)
{
    int clientFd = this->connectedClients[clientName];
    this->connectedClients.erase(clientName);
    for (const std::pair<const std::string, std::set<int>>& group: this->groups)
    {
        group.second.erase(clientFd);
    }
    print_exit(true, clientName);
    return 0;
}


// ------------------------------- server main ------------------------------- //

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        print_server_usage();
    }
    try {
        auto portNum = (unsigned short) std::stoi(argv[1]);
        auto server = new WhatsappServer(portNum);
        bool exit = false;
        fd_set readFds;

        while (!exit)
        {
            // add all Fds to fd_set:
            FD_ZERO(&readFds);
            FD_SET(server->listeningSocket, &readFds);
            FD_SET(STDIN_FILENO, &readFds);
            for (const std::pair<const std::string, int>& client: server->getClients())
            {
                FD_SET(client.second, &readFds);
            }

            if (select(MAX_CLIENTS_NUM +1, &readFds, nullptr, nullptr, nullptr) == -1) {
                print_error("select", errno);
            }

            if (FD_ISSET(server->listeningSocket, &readFds)) { // someone tried to connect (using the listening socket)
                server->establishConnection();
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

            for (const std::pair<const std::string, int>& client: server->getClients())
            {
                if (FD_ISSET(client.second, &readFds))
                {
                    server->readClient(client.first);
                }
            }
        }
        close(server->listeningSocket);
        delete server;
    }
    catch (std::invalid_argument &e)
    {
        print_server_usage();
        exit(1); //todo - ok?
    }
}



