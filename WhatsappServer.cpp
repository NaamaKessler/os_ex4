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
    this->sa.sin_addr.s_addr = INADDR_ANY; //todo
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
 * @return 1 upon success, 0 upon failure.
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
void WhatsappServer::readClient(std::string clientName, int clientFd)
{
    auto buf = new char[WA_MAX_INPUT+1];
    bzero(buf,WA_MAX_INPUT+1);
    int byteCount = 0;
    int byteRead = 0;
    while (byteCount < WA_MAX_INPUT)
    { /* loop until full buffer */
        byteRead = (int) read(clientFd, buf, (WA_MAX_INPUT - byteCount));
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
    std::string message;
    std::vector<std::string> clients;
    parse_command((buf-WA_MAX_INPUT), commandT, name, message, clients);
    int success = 0;
    switch (commandT)
    {
        case CREATE_GROUP:
            success = createGroup(clientName, name, clients);
            echoClient(clientName, clientFd, success);
            break;
        case SEND:
            if (this->groups.find(name) != this->groups.end())
            {
                success = this->sendToGroup(name, clientName, message);
            }
            else
            {
                success = sendMessage(SEND, clientName, clientFd, name, message);
            }
            print_send(true, success, clientName, name, message);
            echoClient(clientName, clientFd, success);
            break;
        case WHO:
            success = whosConnected(clientName);
            echoClient(clientName, clientFd, success);
            break;
        case EXIT:
            exitClient(clientName);
            break;
        case NAME:
            success = insertName(clientFd, name);
            for (auto client: this->connectedClients)
            {
                if ((client.first == DEFAULT_CLIENT_NAME) && (client.second == clientFd))
                {
                    this->connectedClients.erase(clientName);
                    break;
                }
            }
            echoClient(name, clientFd, success);
        default: //we won't get here since Client makes sure that the command type is valid.
            break;
    }
    delete (buf - WA_MAX_INPUT);
}

/**
 * Returns the map of clients and their Fds.
 * @return connected clients map.
 */
std::map<std::string, int> WhatsappServer::getClients()
{
    return this->connectedClients;
}

/**
 * Creates a group of clients.
 * @return 1 upon success, 0 upon failure.
 */
int WhatsappServer::createGroup(std::string& clientName, std::string& groupName,
                                std::vector<std::string>& members)
{
    if ((this->groups.find(groupName) == this->groups.end()) &&
        (this->connectedClients.find(groupName) == this->connectedClients.end()))
    {
        // groupName is unique
        this->groups[groupName];
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

/**
 * Sends a message to all groupName members.
 * @return 1 upon success, 0 upon failure.
 */
int WhatsappServer::sendToGroup(std::string& groupName, std::string& origName, std::string& msg)
{
    if (this->groups.find(groupName) != this->groups.end())
    {
        std::set<std::string> groupMembers = this->groups[groupName];
        if (groupMembers.find(origName) != groupMembers.end())
        {
            for (const std::string& groupMember: groupMembers)
            {
                if (groupMember != origName)
                {
                    if (sendMessage(SEND, origName, this->connectedClients[groupMember], groupMember, msg) == 0)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

/**
 * Sends message fron one client to the another.
 * @return 1 upon success, 0 upon failure.
 */
int WhatsappServer::sendMessage(command_type command, std::string &originName, int origFd,
                                const std::string &destName, std::string &message)
{
    // verify that the client exists:
    if (this->connectedClients.empty() || (this->connectedClients.find(destName) == this->connectedClients
                                                                                           .end()))
    {
        return 0;
    }

    // of it's a message, attach the inner command type & user name according to protocol.
    std::string fullMsg;
    if (command == SEND)
    {
        fullMsg = "receiver " + originName + " " + message;
    }
    else
    {
        fullMsg = message;
    }

    // if connection has failed, we need to send a fail signal to the client, which is currently named '???'.
    // Iterate over the '???' clients and find the one with correct Fd.
    int destination = this->connectedClients.at(destName);
    if (command == NAME && message == "0")
    {
        destination = origFd;
    }
    int byteCount = 0;
    int byteWritten = 0;
    while (byteCount < WA_MAX_INPUT)
    {
        byteWritten = (int)write(destination, fullMsg.c_str() + byteWritten, (size_t)WA_MAX_INPUT-byteCount);
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
    std::vector<std::string> clientsNamesVec;
    std::string connectedClientsNames = "clients ";
    for (const auto& pair: this->connectedClients)
    {
        clientsNamesVec.push_back(pair.first);
    }
    std::sort(clientsNamesVec.begin(), clientsNamesVec.end());
    for (unsigned int i = 0; i < clientsNamesVec.size() - 1; i++)
    {
        connectedClientsNames += clientsNamesVec[i];
        connectedClientsNames += ",";
    }
    connectedClientsNames += clientsNamesVec.back();
    this->sendMessage(INVALID, clientName, this->connectedClients[clientName], clientName,
            connectedClientsNames);
    print_who_server(clientName);
    return 1;
}

/**
 * Executes exit request of the client.
 * @return 1 upon success, 0 upon failure.
 */
int WhatsappServer::exitClient(std::string& clientName)
{
    this->connectedClients.erase(clientName);
    for (auto& group: this->groups)
    {
        group.second.erase(clientName);
        if (group.second.empty()) // last client has left the group
        {
            this->groups.erase(group.first);
        }
    }
    print_exit(true, clientName);
    return 1;
}

/**
 * Receives a new client's name and inserts it in the appropriate pair of this->ConnectedClients
 * (which currently stores a place-saver).
 * @return 1 upon success, 0 upon failure.
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
 */
void WhatsappServer::echoClient(std::string& clientName, int clientFd, int success)
{
    std::string successVal = std::to_string(success);
    this->sendMessage(NAME, clientName, clientFd, clientName, successVal);
}


/**
 * Signals to client that the server has crashed / got an EXIT command.
 */
void WhatsappServer::signalExit(const std::string& clientName)
{
    std::string crash_protocol = "server_crash ";
    sendMessage(INVALID, crash_protocol, this->connectedClients[clientName],  clientName, crash_protocol);
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
                if (client.second > maxClientFd)
                {
                    maxClientFd = client.second;
                }
            }
            // add all Fds to fd_set:
            FD_ZERO(&readFds);
            FD_SET(server->listeningSocket, &readFds);
            FD_SET(STDIN_FILENO, &readFds);
            for (const std::pair<const std::string, int>& client: server->getClients())
            {
                FD_SET(client.second, &readFds);
            }

            if (select(maxClientFd+1, &readFds, nullptr, nullptr, nullptr) == -1) {
                print_error("select", errno);
            }

            if (FD_ISSET(server->listeningSocket, &readFds)) {
                // someone tried to connect (using the listening socket)
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
                if (FD_ISSET(client.second, &readFds))
                {
                    server->readClient(client.first, client.second);
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



