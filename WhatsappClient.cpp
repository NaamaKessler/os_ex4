#include "WhatsappClient.hpp"

//int validateGroup(const std::string& name, const std::vector<std::string>& clients);
//int validateName(const std::string& name);
int validateAddress(char *hostname);
int validatePort(char* port, unsigned short* portNum);

// ------------------------------- private funcs ------------------------------- //

// flow: main calls constructor, which sets socket and connection
// after those are formed, reads user input in a loop
// check if server socket is available, if it is, read/write

// -- initial validations --

int validateAddress(char *hostname){
    std::regex ipTemplate("(\\d{1,3}.){3}(\\d{1,3})");
    if (std::regex_match(hostname, ipTemplate))
    {
        return 0;
    } else {
        return -1;
    }
}

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
    if (intPort < 0 || intPort > MAX_PORT_NUM){
        return -1;
    }

    *portNum = (unsigned short) intPort;
    return 0;
}

// ------------------------------- public funcs ------------------------------- //

// -- post parse validations --

int WhatsappClient::validateName(const std::string& name){
    // groupname, clientname: only letters and  digits
    for (const char& c : name){
        if (!isalnum(c)){
            return -1;
        }
    }
    lastName = name;
    return 0;
}

int WhatsappClient::validateGroup(const std::string& name, const std::vector<std::string>& clients){
    if ((validateName(name) != 0) || (clients.size() < MIN_GROUP_SIZE) || (std::find(clients.begin(),
                                                                                     clients.end(), getClientName()) == clients.end()))
    {
        // is this client in the group
        return -1;
    }
    return 0;
}

int WhatsappClient::validateSend(std::string& receiver){
    return isReceiverNotMe(receiver);
}

//const int WhatsappClient::isGroupMember(std::string& groupName)
//{
//    // should check when sending to group but doesn't know if it's a group/client name on send
//    return std::find(groups.begin(), groups.end(), groupName) != groups.end();
//}

int WhatsappClient::isReceiverNotMe(std::string& receiver){
//    if (strcmp(receiver, getClientName()) != 0){
    if (receiver == getClientName()){
        return -1;
    }
    return 0;
}

// -- --- --

WhatsappClient::WhatsappClient(char* clientName, char* serverAddress, char* serverPort)
{

    // gets: clientName, serverAddress, serverPort
    unsigned short portNum;
    this->myName = clientName;
    if ((validateAddress(serverAddress)) != 0){
        exit(1);
    }

    if ((validatePort(serverPort, &portNum)) != 0){
        exit(1);
    }

    // init socket & connection
    if (this->connectToServer(serverAddress, portNum) != 0)
    {
        print_fail_connection();
        exit(1);
    } else
    { // connected
        print_connection();
    }
}

int WhatsappClient::getSocketHandle()
{
    return this->socketHandle;
}

const std::string WhatsappClient::getClientName()
{
    return this->myName;
}

int WhatsappClient::setLastCommand(command_type command){
    this->lastCommand = command;
    return 0;
}
command_type WhatsappClient::getLastCommand(){
    return lastCommand;
}

//tries to callSocket to the server.
int WhatsappClient::connectToServer(char *hostname, unsigned short portnum)
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
        return -1;
    }

    // connect
    if (connect(socketHandle, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    {
        print_error("callSocket", errno);
        close(socketHandle);
        return -1;
    }
    return 0;
}

int WhatsappClient::parseMsg(std::string msg) //parses the message and calls relevant methods for execution.
{
    command_type commandT;
    std::string name;
    std::string messsage;
    std::vector<std::string> clients;
    parse_command(msg, commandT, name, messsage, clients);
    // validate & call funcs
    lastCommand = commandT;
    lastName = ""; // updated in validateName since every command with name goes through there
    lastClients = clients;
    if (msg == "0" || msg == "1"){
        isLastInnerMsg = true;
        successFromServer = (bool) atoi(msg.c_str());
        return 0;
    }
    switch (commandT){
        case CREATE_GROUP:
            if (validateGroup(name, clients) != 0){
                return -1;
            }
            break;
        case SEND:
            if (validateSend(name) != 0){
                return -1;
            }
            break;
        case WHO:
            break;
        case EXIT:
            break;
        case NAME:
            if (validateName(name) != 0) {
                return -1;
            }
            break;
        case SERVER_CRASH:
            // server exits before the client
            close(this->getSocketHandle());
            exit(1); // todo: what exit code?
        case INVALID:
            print_invalid_input();
            break;
    }
    return 0;
}

int WhatsappClient::readFromServer()
{
//    char* readBuffer;
    auto readBuffer = new char[MAX_MESSAGE_LEN+1];
    bzero(readBuffer,MAX_MESSAGE_LEN+1);
    int totalBytesRead = 0; //counts bytes read
    int bytesRead = 0; // bytes read this pass
    while (totalBytesRead < MAX_MESSAGE_LEN)
    { /* loop until full buffer */
        std::cout << "in while" << std::endl;
        bytesRead = (int) read(socketHandle, readBuffer, (MAX_MESSAGE_LEN - totalBytesRead));
        if (bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            readBuffer += bytesRead;
        }
        std::cout << "totalBytesRead" << totalBytesRead << std::endl;
        if (readBuffer == '\0'){
            break;
        }

        if (bytesRead < 1)
        {
            return (-1);
        }

    }
    std::cout << "out of while" << std::endl;

    if (parseMsg(readBuffer-MAX_MESSAGE_LEN) != 0) // updates lastCommand, lastName, lastClients
    {
        return -1;
    }
    delete (readBuffer-MAX_MESSAGE_LEN);
    return totalBytesRead;
}

//int WhatsappClient::clientOutput(command_type commandT, std::string name, std::vector<std::string> clients,
//                                 bool success){
int WhatsappClient::clientOutput(){

    switch (lastCommand){

        case CREATE_GROUP:
            print_create_group(false, successFromServer, nullptr, lastName);
            break;
        case SEND:
            print_send(false, successFromServer, nullptr, nullptr, nullptr);
            break;
        case WHO:
            print_who_client(successFromServer, lastClients);
            break;
        case NAME: // just connected
            // handled in init - prints connection msg
            break;
        case EXIT:
            print_exit(false, nullptr);
            break;
        case INVALID:
            //shouldn't get here - already prints invalid input
            break;
        default:
            // shouldn't get here
            break;
    }
    return 0;
}

int WhatsappClient::writeToServer(std::string msg) //needed? (writea according to the protocol)
{
//    char* writeBuffer;
    auto writeBuffer = new char[MAX_MESSAGE_LEN+1];
    bzero(writeBuffer,MAX_MESSAGE_LEN+1);
    if (parseMsg(msg) != 0)
    {
        return -1;
    }
    strcpy(writeBuffer,msg.c_str());
    writeBuffer[msg.length()] = '\0';
    int totalBytesWritten = 0; //counts bytes read
    int bytesWritten = 0; // bytes read this pass

    while (totalBytesWritten < MAX_MESSAGE_LEN)
    { /* loop until full buffer */
        bytesWritten = (int) write(socketHandle, writeBuffer, (MAX_MESSAGE_LEN - totalBytesWritten));
        if (bytesWritten > 0)
        {
            totalBytesWritten += bytesWritten;
//            writeBuffer += bytesWritten;
        }

        if (bytesWritten < 1)
        {
            return (-1);
        }

    }
//    std::cout << "totalBytesWritten: " << totalBytesWritten << std::endl;
//    std::cout << "writeBuffer: " << writeBuffer << std::endl;

    delete writeBuffer;
    return totalBytesWritten;

}

// ------------------------------- Client main  ------------------------------- //


int main(int argc, char* argv[]){
    fd_set rfds;
//    struct timeval tv;
    int retval;
    bool exitFlag = false;

    // usage error:
    if (argc != NUM_OF_ARGS){
        print_client_usage();
        // exit
        exit(1);
    }

    // init socket & connection
    WhatsappClient whatsappClient = WhatsappClient(argv[1], argv[2], argv[3]);
    std::string nameCommand = "name ";
    nameCommand.append(argv[1]);
    whatsappClient.writeToServer(nameCommand);

    // wait for input
    std::string inputLine;

    while (!exitFlag){
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(whatsappClient.getSocketHandle(), &rfds);
        // get input from user

        // needs to get success message from the server - so it can print a success message

        retval = select(whatsappClient.getSocketHandle()+1, &rfds, nullptr, nullptr, nullptr);
        if (retval == -1)
        {
            //error
        }
        else
        {
            if (FD_ISSET(STDIN_FILENO, &rfds))
            { // from stdin
                std::cout << "in stdin" << std::endl;
//                auto readBuffer = new char[MAX_MESSAGE_LEN+1];
//                bzero(readBuffer,MAX_MESSAGE_LEN+1);
//                int totalBytesRead = 0; //counts bytes read
//                int bytesRead = 0; // bytes read this pass
//                while (totalBytesRead < MAX_MESSAGE_LEN)
//                { /* loop until full buffer */
//                    bytesRead = (int) read(STDIN_FILENO, readBuffer, (MAX_MESSAGE_LEN - totalBytesRead));
//                    if (bytesRead > 0)
//                    {
//                        totalBytesRead += bytesRead;
//                        readBuffer += bytesRead;
//                    }
//
//                    if (bytesRead < 1)
//                    {
//                        return (-1);
//                    }
//
//                }
                getline(std::cin, inputLine);
//                std::cout << "readBuffer: " << readBuffer-MAX_MESSAGE_LEN << std::endl;
//                if (strcmp(readBuffer-MAX_MESSAGE_LEN, "exit") || strcmp(readBuffer-MAX_MESSAGE_LEN, "EXIT"))
                if ((inputLine == "exit") || inputLine == "EXIT")
                {
                    std::cout << "in exit" << std::endl;
                    exitFlag = true;
//            whatsappClient.parseMsg(inputLine); // validation
                    whatsappClient.writeToServer(inputLine);
                    continue;
                }
                whatsappClient.parseMsg(inputLine); // validation
                whatsappClient.writeToServer(inputLine);
            }
            if (FD_ISSET(whatsappClient.getSocketHandle(), &rfds)) // from server
            {
                std::cout << "listening to " << whatsappClient.getSocketHandle() << std::endl;
                if (whatsappClient.readFromServer() != -1){ // calls parseMsg
                    if (whatsappClient.isLastInnerMsg){ // enter here only if it got 0/-1

                        whatsappClient.clientOutput();
                        whatsappClient.isLastInnerMsg = false;
                    }
                } // calls parseMsg
            }

        }

    }
    close(whatsappClient.getSocketHandle()); // todo - is needed elsewhere?
    exit(0);

}
