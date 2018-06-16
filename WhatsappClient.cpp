//
// Created by Naama on 6/12/2018.
//
#include "WhatsappClient.hpp"

int validateGroup(const std::string& name, const std::vector<std::string>& clients);
int validateName(const std::string& name);
int validateAddress(char *hostname);
int validatePort(unsigned short portNum);

//todo: if the server exits before the client should exit(1) without crashing.

// ------------------------------- private funcs ------------------------------- //

// flow: main calls constructor, which sets socket and connection
// after those are formed, reads user input in a loop
// check if server socket is available, if it is, read/write

int validatePort(char* port, unsigned short* portNum){

    if (isdigit(*port) != 0)
    {
        return -1;
    }
    else { *portNum = (unsigned short) *port; }
    if (portNum < 0 || *portNum > MAX_PORT_NUM){
        return -1;
    }
    return 0;
}

int validateName(const std::string& name){
    // groupname, clientname: only letters and  digits
    for (const char& c : name){
        if (!isalnum(c)){
            return -1;
        }
    }
    return 0;
}

int validateGroup(const std::string& name, const std::vector<std::string>& clients){
    if (validateName(name) != 0)
    {
        return -1;
    }
    if (clients.size() < MIN_GROUP_SIZE)
    {
        return -1;
    }
    return 0;
}

int validateAddress(char *hostname){
    std::regex ipTemplate("(\\d{1,3}\\.){3}(\\d{1,3})");
    if (std::regex_match(hostname, ipTemplate) != 0)
    {
        return -1;
    } else {
        return 0;
    }
}

// ------------------------------- public funcs ------------------------------- //

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

//tries to callSocket to the server.
int WhatsappClient::connectToServer(char *hostname, unsigned short portnum)
{
    // init
    if ((hp=gethostbyname(hostname)) == NULL){
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    memcpy((char*)&sa.sin_addr, hp->h_addr, (size_t)hp->h_length);
    sa.sin_family = (sa_family_t ) hp->h_addrtype;
    sa.sin_port=htons((u_short) portnum);

    // socket
    if ((socketHandle=socket(hp->h_addrtype, SOCK_STREAM,0))<0){
        return -1;
    }

    // connect
    if (connect(socketHandle, (struct sockaddr *) &sa, sizeof(sa)) < 0){
        close(socketHandle);
        return -1;
    }

}

int WhatsappClient::parseMsg(std::string msg) //parses the message and calls relevant methods for execution.
{
    command_type commandT;
    std::string name;
    std::string messsage;
    std::vector<std::string> clients;
    parse_command(msg, commandT, name, messsage, clients);
    // validate & call funcs
    switch (commandT){
        case CREATE_GROUP:
            if (validateGroup(name, clients) != 0){
                return -1;
            }
            break;
        case SEND:
            break;
        case WHO:
            break;
        case EXIT:
            break;
        case INVALID:
            print_invalid_input();
            break;
    }

}

// makes sure the input is valid
// before it will be sent to the server.
//int WhatsappClient::validateMsg(const command_type& commandT, const std::string& name, const std::string&
//message, const std::vector<std::string>& clients)
//{
//    // validate ip address
//    switch (commandT){
//        case CREATE_GROUP:
//            if (validateGroup(name, clients) != 0){
//                return -1;
//            }
//            break;
//        case SEND:
//            break;
//        case WHO:
//            break;
//        case EXIT:
//            break;
//        case INVALID:
//            print_invalid_input();
//            break;
//    }
//
//    return 0;
//    // tcp based connections
//    // serverAddress is an IP address
//}

int WhatsappClient::readFromServer()
{
    int totalBytesRead = 0; //counts bytes read
    int bytesRead = 0; // bytes read this pass

    while (totalBytesRead < MAX_MESSAGE_LEN)
    { /* loop until full buffer */
        bytesRead = (int) read(socketHandle, readBuffer, (MAX_MESSAGE_LEN - totalBytesRead));
        if (bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            readBuffer += bytesRead;
        }

        if (bytesRead < 1)
        {
            return (-1);
        }

    }
}

int WhatsappClient::writeToServer(std::string msg) //needed? (writea according to the protocol)
{
    if (parseMsg(msg) != 0)
    {
        return -1;
    }
    strcpy(writeBuffer,msg.c_str());
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

}

int WhatsappClient::getSocketHandle()
{
    return this->socketHandle;
}

int main(int argc, char* argv[]){
    fd_set rfds;
    struct timeval tv;
    int retval;

    // usage error:
    if (argc != NUM_OF_ARGS){
        print_client_usage();
        // exit
        exit(1);
    }

    // init socket & connection
    WhatsappClient c = WhatsappClient(argv[1], argv[2], argv[3]);

    // wait for input
    char* input;
    FD_ZERO(&rfds);
    FD_SET(c.getSocketHandle(), &rfds);
    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while (true){ // change condition //todo
        // get input from user
        std::fgets(input,MAX_MESSAGE_LEN,stdin); // todo is it the right len for this
        c.parseMsg(input); // validation
        retval = select(c.getSocketHandle(), &rfds, nullptr, nullptr, &tv); //socketHandle + 1?
        if (retval == -1)
        {
            //error
        }
        else if (retval)
        {
            // when to do read and write? todo
            c.writeToServer(input);
            c.readFromServer();
        }
        else
        {
            // timeout
        }
        break; // break if exit
    }

    exit(0);

}
