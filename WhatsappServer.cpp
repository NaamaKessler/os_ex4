// ------------------------------- includes ------------------------------- //
#include "WhatsappServer.hpp"


// ------------------------------- public methods ------------------------------- //
/**
 * initializes the server and opens a general requests socket.
 */
WhatsappServer::WhatsappServer(unsigned short portNum)
{
    // init hostent
    if (gethostname(this->myName, 30) < 0) // macro raises error -?
    {
        print_error("gethostname", errno);
    }
    this->hp = gethostbyname(this->myName);
    if (!this->hp)
    {
        print_error("gethostbyname", errno);
    }

    // init sockets address
    memset(&this->sa, 0, sizeof(this->sa));
    sa.sin_family = (sa_family_t)hp->h_addrtype;
    memcpy(&sa.sin_addr, hp->h_addr, (size_t)hp->h_length);
    sa.sin_port = htons(portNum);

    // init and bind listening socket
    if (((this->listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0))
    {
        print_error("socket", errno);
    }

    if (bind(this->listeningSocket, (struct sockaddr*)&this->sa, sizeof (struct sockaddr_in)) < 0)
    {
        print_error("bind", errno);
        close(this->listeningSocket);
    }

    if (listen(this->listeningSocket, 10) < 0)
    {
        print_error("listen", errno);
    }
}

/**
 * Destructor.
 */
WhatsappServer::~WhatsappServer()
{
    close(this->listeningSocket);
    for (const std::pair<const std::string, int>& client: this->connectedClients)
    {
        signalExit(client.first);
    }
}

/**
 * Accepts a connection request and opens a socket for communication with the client.
 */
int WhatsappServer::establishConnection()
{
    int newSockFd = accept(this->listeningSocket, nullptr, nullptr);
    if (newSockFd < 0)
    {
        print_fail_connection();
        return 0;
    }
    else
    {
        // insert to Fds map (register to server).
        this->connectedClients.insert(std::pair<std::string, int>(DEFAULT_CLIENT_NAME, newSockFd));
        return 1;
    }
}

/**
 * Reads messages from the client and carries them out.
 * @return
 */
void WhatsappServer::readClient(std::string clientName)
{
    int clientFd = this->connectedClients[clientName];
    auto buf = new char[257];
    bzero(buf,257);
//    memset(buf, '\0', 257);
    int byteCount = 0;
    int byteRead = 0;
//    byteRead = (int)recv(clientFd, buf,  ((size_t)256-byteCount), 0);
    while (byteCount < 256)
    { /* loop until full buffer */
        byteRead = (int) read(clientFd, buf, (256 - byteCount));
        if (byteRead > 0)
        {
            byteCount += byteRead;
            buf += byteRead;
        }

        if (byteRead < 1)
        {
//            exit(1);
            print_error("read", errno);
        }

    }
//    std::cout << "out of while: " << (buf-256) << std::endl;
    // parse command:
    command_type commandT;
    std::string name;
    std::string message;
    std::vector<std::string> clients;
    parse_command((buf-256), commandT, name, message, clients);
    int success = 0;
    switch (commandT)
    {
        case CREATE_GROUP:
            success = createGroup(clientName, name, clients);
            echoClient(clientName, success);
            break;
        case SEND:
            if (this->groups.find(name) != this->groups.end())
            {
                success = this->sendToGroup(name, clientName, message);
            }
            else
            {
                success = sendMessage(SEND, clientName, name, message);
            }
            print_send(true, true, clientName, name, message);
            echoClient(clientName, success);
            break;
        case WHO:
            success = whosConnected(clientName);
//            std::cout << "success: " << success << std::endl;
            echoClient(clientName, success);
            break;
        case EXIT:
            exitClient(clientName);
//            std::cout << "finished EXIT" << std::endl;
            break;
        case NAME:
            insertName(clientFd, name);
        default: //we won't get here since Client makes sure that the command type is valid.
            break;
    }

    delete (buf - 256);
}

/**
 * Returns the map of clients and their Fds.
 * @return
 */
std::map<std::string, int> WhatsappServer::getClients()
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
                this->groups[groupName].insert(member);
                // set ensures that no member will be inserted twice.
            }
            else
            {
                print_create_group(true, false, clientName, groupName);
                this->groups.erase(groupName);
                return 0;
            }
        }
        this->groups[groupName].insert(clientName); //add creator to group
        print_create_group(true, true, clientName, groupName);
        return 1;
    }
    else  // group name already exists
    {
        print_create_group(true, false, clientName, groupName);
        return 0;
    }
}


// ------------------------------- private methods ------------------------------- //

int WhatsappServer::sendToGroup(std::string& groupName, std::string& origName, std::string& msg)
{
    if (this->groups.find(groupName) != this->groups.end())
    {
        std::set<std::string> clients = this->groups[groupName];
        for (const std::string& client: clients)
        {
            if (client != origName)
            {
//                std::cout << "client: " << client << std::endl;
                if (sendMessage(SEND, origName, client, msg) == 0)
                {
                    // error
                    return 0;
                }
            }
        }

        return 1;
    }
    else
    {
        //error
        return 0;
    }
}

/**
 * Sends message fron one client to the another.
 * @return
 */
int WhatsappServer::sendMessage(command_type command, std::string &originName, const std::string &destName,
                                std::string &message)
{
    // verify that the client exists:
    if (this->connectedClients.empty()) //todo - check client exists
    {
        return 0;
    }
    std::string fullMsg;
//    std::cout << "message: " << message << std::endl;
    if (command == SEND)
    {
        fullMsg = "receiver " + originName + " " + message;
    }
    else
    {
        fullMsg = message;
    }
    int byteCount = 0;
    int byteWritten = 0;
    while (byteCount < 256)
    {
        byteWritten = (int)write(this->connectedClients.at(destName),
                                 fullMsg.c_str() + byteWritten, (size_t)256-byteCount);
//        std::cout << "byteWritten: " << byteWritten << std::endl;
        if (byteWritten > 0)
        {
            byteCount += byteWritten;
        }
        else
        {
            print_error("write", errno);
            return 0;
        }
    }
    return 1;
}

/**
 * @return a list containing all connected clients names.
 */
int WhatsappServer::whosConnected(std::string& clientName)
{
//    auto connectedClientsNames = new char[(30*this->connectedClients.size()) + 1];
    std::vector<std::string> clientsNamesVec;
    std::string connectedClientsNames = "clients ";
//    memset(connectedClientsNames, '\0', (30*this->connectedClients.size()) + 1);
    for (const auto& pair: this->connectedClients)
    {
//        memcpy(connectedClientsNames, (pair.first).c_str() + ',', pair.first.length());  //todo - sort, comma
//        connectedClientsNames += pair.first;
        clientsNamesVec.push_back(pair.first);
    }
    std::sort(clientsNamesVec.begin(), clientsNamesVec.end());
    for (int i = 0; i < clientsNamesVec.size() - 1; i++)
    {
        connectedClientsNames += clientsNamesVec[i];
        connectedClientsNames += ",";
    }
    connectedClientsNames += clientsNamesVec.back();
    this->sendMessage(INVALID, clientName, clientName, connectedClientsNames);
    print_who_server(clientName);
    return 1;
}

/**
 * Executes exit request of the client.
 * @return
 */
int WhatsappServer::exitClient(std::string& clientName)
{
    this->connectedClients.erase(clientName);
    for (auto& group: this->groups)
    {
//        std::cout << "delete from group" << std::endl;
        group.second.erase(clientName);
    }
//    std::cout << "after for" << std::endl;
    print_exit(true, clientName);
    return 1;
}

/**
 * Receives a new client's name and inserts it in the appropriate pair of this->ConnectedClients
 * (which currently stores a place-saver).
 */
int WhatsappServer::insertName(int clientFd, std::string& name)
{
    // make sure name is not in use already:
    if (this->connectedClients.find(name) != this->connectedClients.end())
    {
        print_dup_connection();
        return 0;
    }

    // connect fd to name:
    for (auto& client: this->connectedClients)
    {
        if ((client.first == DEFAULT_CLIENT_NAME) && client.second == clientFd)
        {
            int fd = client.second;
            this->connectedClients.erase(client.first);
            this->connectedClients[name] = fd;
            break;
        }
    }
    print_connection_server(name);
    return 1;
}

/**
 * Signals the client that the request has succeeded/failed.
 * @param clientFd
 * @param success
 */
void WhatsappServer::echoClient(std::string& clientName, int success)
{
    std::string successVal = std::to_string(success);
//    unsigned int byteCount = 0;
//    unsigned int byteWritten = 0;
//    while (byteCount < sizeof(int))
//    {
//        byteWritten = (unsigned int)write(clientFd, (char*)(&success), 4);
//        if (byteWritten > 0)
//        {
//            byteCount += byteWritten;
//        }
//        else
//        {
//            print_error("write", errno);
//        }
//    }
    this->sendMessage(INVALID, clientName, clientName, successVal);
}


/**
 * Signals to client that the server has crashed / got an EXIT command.
 */
void WhatsappServer::signalExit(const std::string& clientName)
{
    std::string crash_protocol = "server_crash ";
//    unsigned int byteCount = 0;
//    unsigned int byteWritten = 0;
//    while (byteCount < sizeof(char)*crash_protocol.length())
//    {
//        byteWritten = (int)write(clientFd, crash_protocol.c_str() + byteWritten, sizeof(int));
//        if (byteWritten > 0)
//        {
//            byteCount += byteWritten;
//        }
//        else
//        {
//            print_error("write", errno);
//        }
    sendMessage(INVALID, crash_protocol, clientName, crash_protocol);
//    }
}


// ------------------------------- server main ------------------------------- //

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        print_server_usage();
        exit(1);
    }
    try {
        auto portNum = (unsigned short) std::stoi(argv[1]);
        auto server = new WhatsappServer(portNum);
        bool exitFlag = false;
        fd_set readFds;

        int maxClientFd = server->listeningSocket;

        while (!exitFlag)
        {
            // find max fd:
            for (const std::pair<const std::string, int>& client: server->getClients())
            {
//                std::cout << "for 1" << std::endl;
                if (client.second > maxClientFd)
                {
                    maxClientFd = client.second;
                }
            }
//            std::cout << "max fd: " << maxClientFd << std::endl;
//            std::cout << server->listeningSocket << std::endl;
            // add all Fds to fd_set:
            FD_ZERO(&readFds);
            FD_SET(server->listeningSocket, &readFds);
            FD_SET(STDIN_FILENO, &readFds);
            for (const std::pair<const std::string, int>& client: server->getClients())
            {
//                std::cout << "for 2" << std::endl;
                FD_SET(client.second, &readFds);
            }

            if (select(maxClientFd+1, &readFds, nullptr, nullptr, nullptr) == -1) {
                print_error("select", errno);
            }

            if (FD_ISSET(server->listeningSocket, &readFds)) { // someone tried to connect (using the listening socket)
                server->establishConnection();
                continue;
            }
            if (FD_ISSET(STDIN_FILENO, &readFds))
            {
                std::string inputLine;
                getline(std::cin, inputLine);
                if (strcmp(inputLine.c_str(), "EXIT") == 0)
                {
                    exitFlag = true;
                    print_exit();
                }
            }

            for (const std::pair<const std::string, int> &client: server->getClients())
            {
//                std::cout << "for 3" << std::endl;
                if (FD_ISSET(client.second, &readFds))
                {
                    server->readClient(client.first);
                }
            }

        }
        // clean-ups and exit:
        for (const std::pair<const std::string, int>& client: server->getClients())
        {
            server->signalExit(client.first);
        }
        close(server->listeningSocket);
        delete server;
        exit(0);
    }
    catch (std::invalid_argument &e)
    {
        print_server_usage();
        exit(1);
    }
}



