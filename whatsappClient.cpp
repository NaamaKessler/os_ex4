#include "whatsappClient.hpp"

int validateAddress(char *hostname);
int validatePort(char* port, unsigned short* portNum);

// ------------------------------- usage funcs ------------------------------- //

// flow: main calls constructor, which sets socket and connection
// after those are formed, reads user input in a loop
// check if server socket is available, if it is, read/write

// -- initial validations --

/**
 * validates address.
 * @param hostname
 * @return 0 for success, -1 for failure.
 */
int validateAddress(char *hostname){
    std::regex ipTemplate("(\\d{1,3}.){3}(\\d{1,3})");
    if (std::regex_match(hostname, ipTemplate))
    {
        return 0;
    } else {
        return -1;
    }
}

/**
 * validates port.
 * @param port
 * @param portNum
 * @return 0 for success, -1 for failure.
 */
int validatePort(char* port, unsigned short* portNum){

    int intPort;
    std::string strPort = std::string(port);
    // check if digits
    for (char &c : strPort){
        if (isdigit(c) == 0)
        {
            return -1;
        }
    }
    // convert to int and check range
    intPort = atoi(port);
    if (intPort < MIN_PORT_NUM || intPort > MAX_PORT_NUM){
        return -1;
    }

    *portNum = (unsigned short) intPort;
    return 0;
}

// ------------------------------- public funcs ------------------------------- //

// -- post parse validations --
/**
 * validates name.
 * @param name
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::validateName(const std::string& name){
    // groupname, clientname: only letters and  digits
    for (const char& c : name){
        if (!isalnum(c)){
            return -1;
        }
    }
    lastName = name;
    return 0;
}

/**
 * validates group.
 * @param name
 * @param clients
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::validateGroup(const std::string& name, const std::vector<std::string>& clients){
    if ((validateName(name) != 0) || (clients.size() < MIN_GROUP_SIZE) || (clients.size() == 1 && clients
          .back() == myName))
    {
        return -1;
    }
    return 0;
}

/**
 * validates send command.
 * @param receiver
 * @param message
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::validateSend(std::string& receiver, std::string& message){
    return (isReceiverNotMe(receiver) || (message.size() > MAX_MESSAGE_LEN));
}

/**
 * Checks if the receiver is not the sender.
 * @param receiver
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::isReceiverNotMe(std::string& receiver){
    if (receiver == getClientName()){
        return -1;
    }
    return 0;
}

// -- --- --
/**
 * WhatsappClient constructor.
 */
whatsappClient::whatsappClient(char* clientName, char* serverAddress, char* serverPort)
{
    unsigned short portNum;
    this->myName = clientName;
    this->initalized = false;
    if ((validateAddress(serverAddress)) != 0){
        print_client_usage();
        exit(1);
    }

    if ((validatePort(serverPort, &portNum)) != 0){
        print_client_usage();
        exit(1);
    }

    // init socket & connection
    if (this->connectToServer(serverAddress, portNum) != 0)
    {
        print_fail_connection();
        exit(1);
    }
}

/**
 * WhatsappClient destructor.
 */
whatsappClient::~whatsappClient(){
    close(this->getSocketHandle());
}

/**
 * @return socket fd.
 */
int whatsappClient::getSocketHandle()
{
    return this->socketHandle;
}

/**
 * @return the client's name.
 */
const std::string whatsappClient::getClientName()
{
    return this->myName;
}

/**
 * Sets last command's type.
 */
int whatsappClient::setLastCommand(command_type command){
    this->lastCommand = command;
    return 0;
}

/**
 * Sets lastname.
 */
int whatsappClient::setLastName(std::string& name){
    this->lastName = name;
    return 0;
}

/**
 * tries to callSocket to the server.
 * @param hostname
 * @param portnum
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::connectToServer(char *hostname, unsigned short portnum)
{
    // init
    if ((hp = gethostbyname(hostname)) == NULL)
    {
        print_error("gethostbyname", errno);
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    memcpy((char *) &sa.sin_addr, hp->h_addr, (size_t) hp->h_length);
    sa.sin_family = (sa_family_t) hp->h_addrtype;
    sa.sin_port = htons(portnum);


    // socket
    if ((socketHandle = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
    {
        print_error("callSocket", errno);
        exit(1);
//        return -1;
    }

    // connect
    if (connect(socketHandle, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    {
        print_error("callSocket", errno);
        close(socketHandle);
        exit(1);
    }
    return 0;
}

/**
 * Gets input from user and parses it.
 * @param msg
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::inputFromUser(std::string msg)
{
    command_type commandT;
    std::string name;
    std::string message;
    std::vector<std::string> clients;
    parse_command(msg, commandT, name, message, clients);
    lastCommand = commandT;
    lastName = ""; // updated in validateName since every command with name goes through there
    lastClients = clients;
    switch (commandT){
        case CREATE_GROUP:
            if (validateGroup(name, clients) != 0){
                print_create_group(false, false, myName, name);
                return -1;
            }
            break;
        case SEND:
            lastName = name;
            if (validateSend(name, message) != 0){
                print_send(false, false, myName, name, message);
                return -1;
            }
            break;
        case WHO:
            break;
        case EXIT:
            break;
        case NAME:
            break;
        case SERVER_CRASH:
            break;
        case CLIENTS:
            break;
        case RECEIVER:
            break;
        case INVALID:
            print_invalid_input();
            break;
        default:
            break;
    }

    return 0;
}

/**
 * Gets input from the server and parses it, and prints to the screen.
 * @param msg
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::inputFromServer(std::string msg)
{
    command_type commandT;
    std::string name;
    std::string message;
    std::vector<std::string> clients;
    if (msg == "0" || msg == "1"){
        successFromServer = (bool) atoi(msg.c_str());
        clientOutput(lastCommand, lastName, message, lastClients, successFromServer);
        return 0;
    } else
    {
        parse_command(msg, commandT, name, message, clients);
        lastCommand = commandT;
        lastName = ""; // updated in validateName since every command with name goes through there
        lastClients = clients;
        validateName(name);
        if (lastCommand == RECEIVER || lastCommand == SERVER_CRASH){ // commands that don't need success
            clientOutput(lastCommand, lastName, message, lastClients, successFromServer);
        }
    }
    return 0;
}

/**
 * Reads the input from the server.
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::readFromServer()
{
    auto readBuffer = new char[WA_MAX_INPUT+1];
    bzero(readBuffer,WA_MAX_INPUT+1);
    int totalBytesRead = 0; //counts bytes read
    int bytesRead = 0; // bytes read this pass
    while (totalBytesRead < WA_MAX_INPUT)
    { /* loop until full buffer */
        bytesRead = (int) read(socketHandle, readBuffer, (WA_MAX_INPUT - totalBytesRead));

        if (bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            readBuffer += bytesRead;
        }
        if (readBuffer == '\0'){
            break;
        }

        if (bytesRead < 1)
        {
            print_error("read", errno);
            return (-1);
        }

    }

    if (inputFromServer(readBuffer-WA_MAX_INPUT) != 0) // updates lastCommand, lastName, lastClients
    {
        return -1;
    }
    delete (readBuffer-WA_MAX_INPUT);
    return totalBytesRead;
}

/**
 * Prints the relevant message to the screen.
 * @param commandT
 * @param name
 * @param message
 * @param clients
 * @param success
 * @return 0 for success, -1 for failure.
 */
int whatsappClient::clientOutput(command_type commandT, std::string name, std::string& message,
                                 std::vector<std::string> clients,
                                 bool success){
    switch (commandT){

        case CREATE_GROUP:
            print_create_group(false, success, name, name);
            break;
        case SEND:
            print_send(false, success, name, name, message);
            break;
        case WHO:
            break;
        case NAME: // just connected
            // handled in init - prints connection msg
            if (successFromServer)
            {
                print_connection();
            } else {
                print_dup_connection();
                close(this->getSocketHandle());
                exit(1);
            }
            break;
        case EXIT:
            print_exit(false, name);
            break;
        case CLIENTS:
            print_who_client(success, clients);
            break;
        case RECEIVER:
            print_message(name, message);
            break;
        case INVALID:
            //shouldn't get here - already prints invalid input
            break;
        case SERVER_CRASH:
            close(this->getSocketHandle());
            exit(1);
        default:
            // shouldn't get here
            break;
    }
    return 0;
}

/**
 * Writes to the server.
 * @param msg
 * @return totalBytesWritten
 */
int whatsappClient::writeToServer(std::string msg)
{
    auto writeBuffer = new char[WA_MAX_INPUT+1];
    bzero(writeBuffer,WA_MAX_INPUT+1);
    if (initalized && (inputFromUser(msg) != 0))
    {
        return -1;
    }
    strcpy(writeBuffer,msg.c_str());
    writeBuffer[msg.length()] = '\0';
    int totalBytesWritten = 0; //counts bytes read
    int bytesWritten = 0; // bytes read this pass

    while (totalBytesWritten < WA_MAX_INPUT)
    { /* loop until full buffer */
        bytesWritten = (int) write(socketHandle, writeBuffer, (WA_MAX_INPUT - totalBytesWritten));
        if (bytesWritten > 0)
        {
            totalBytesWritten += bytesWritten;
        }

        if (bytesWritten < 1)
        {
            print_error("write", errno);
            return (-1);
        }

    }

    delete writeBuffer;
    return totalBytesWritten;

}

// ------------------------------- Client main  ------------------------------- //


int main(int argc, char* argv[]){
    fd_set rfds;
    int retval;
    bool exitFlag = false;

    // usage error:
    if (argc != NUM_OF_ARGS){
        print_client_usage();
        // exit
        exit(1);
    }

    // init socket & connection
    whatsappClient* client = new whatsappClient(argv[1], argv[2], argv[3]);
    std::string nameCommand = "name ";
    std::string name = argv[1];
    client->validateName(name);
    nameCommand.append(name);
    client->setLastCommand(NAME);
    client->setLastName(name);
    client->writeToServer(nameCommand);
    client->initalized = true;

    // wait for input
    std::string inputLine;

    while (!exitFlag){
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(client->getSocketHandle(), &rfds);
        // get input from user

        // needs to get success message from the server - so it can print a success message

        retval = select(client->getSocketHandle()+1, &rfds, nullptr, nullptr, nullptr);

        if (retval == -1)
        {
            print_error("select", errno);
        }
        else
        {
            if (FD_ISSET(STDIN_FILENO, &rfds)) // from user
            {
                getline(std::cin, inputLine);
                if (inputLine == "exit")
                {
                    exitFlag = true;
                    client->writeToServer(inputLine);
                    continue;
                }
                client->writeToServer(inputLine);
            }
            if (FD_ISSET(client->getSocketHandle(), &rfds)) // from server
            {
                client->readFromServer();
            }
        }
    }
    client->readFromServer();


    delete(client);
    exit(0);

}
