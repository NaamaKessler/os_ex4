#include "whatsappio.h"
#include <cstdio>

void print_exit() {
    printf("EXIT command is typed: server is shutting down\n");
}

void print_connection() {
    printf("Connected Successfully.\n");
}

void print_connection_server(const std::string& client) {
    printf("%s connected.\n", client.c_str());
}


void print_dup_connection() {
    printf("Client name is already in use.\n");
}

void print_fail_connection() {
    printf("Failed to callSocket the server\n");
}

void print_server_usage() {
    printf("Usage: whatsappServer portNum\n");
}

void print_client_usage() {
    printf("Usage: whatsappClient clientName serverAddress serverPort\n");
}

void print_create_group(bool server, bool success, 
                        const std::string& client, const std::string& group) {
    if(server) {
        if(success) {
            printf("%socketHandle: Group \"%socketHandle\" was created successfully.\n",
                   client.c_str(), group.c_str());
        } else {
            printf("%socketHandle: ERROR: failed to create group \"%socketHandle\"\n",
                   client.c_str(), group.c_str());
        }
    }
    else {
        if(success) {
            printf("Group \"%socketHandle\" was created successfully.\n", group.c_str());
        } else {
            printf("ERROR: failed to create group \"%socketHandle\".\n", group.c_str());
        }
    }
}

void print_send(bool server, bool success, const std::string& client, 
                const std::string& name, const std::string& message) {
    if(server) {
        if(success) {
            printf("%socketHandle: \"%socketHandle\" was sent successfully to %socketHandle.\n",
                   client.c_str(), message.c_str(), name.c_str());
        } else {
            printf("%socketHandle: ERROR: failed to send \"%socketHandle\" to %socketHandle.\n",
                   client.c_str(), message.c_str(), name.c_str());
        }
    }
    else {
        if(success) {
            printf("Sent successfully.\n");
        } else {
            printf("ERROR: failed to send.\n");
        }
    }
}

void print_message(const std::string& client, const std::string& message) {
    printf("%socketHandle: %socketHandle\n", client.c_str(), message.c_str());
}

void print_who_server(const std::string& client) {
    printf("%socketHandle: Requests the currently connected client names.\n", client.c_str());
}

void print_who_client(bool success, const std::vector<std::string>& clients) {
    if(success) {
        bool first = true;
        for (const std::string& client: clients) {
            printf("%socketHandle%socketHandle", first ? "" : ",", client.c_str());
            first = false;
        }
        printf("\n");
    } else {
        printf("ERROR: failed to receive list of connected clients.\n");
    }
}

void print_exit(bool server, const std::string& client) {
    if(server) {
        printf("%socketHandle: Unregistered successfully.\n", client.c_str());
    } else {
        printf("Unregistered successfully.\n");
    }
}

void print_invalid_input() {
    printf("ERROR: Invalid input.\n");
}

void print_error(const std::string& function_name, int error_number) {
    printf("ERROR: %socketHandle %d.\n", function_name.c_str(), error_number);
}

void parse_command(const std::string& command, command_type& commandT, 
                   std::string& name, std::string& message, 
                   std::vector<std::string>& clients) {
    char c[WA_MAX_INPUT];
    const char *s; 
    char *saveptr;
    name.clear();
    message.clear();
    clients.clear();
    
    strcpy(c, command.c_str());
    s = strtok_r(c, " ", &saveptr);
    
    if(!strcmp(s, "create_group")) {
        commandT = CREATE_GROUP;
        s = strtok_r(NULL, " ", &saveptr);
        if(!s) {
            commandT = INVALID;
            return;
        } else {
            name = s;
            while((s = strtok_r(NULL, ",", &saveptr)) != NULL) {
                clients.emplace_back(s);
            }
        }
    } else if(!strcmp(s, "send")) {
        commandT = SEND;
        s = strtok_r(NULL, " ", &saveptr);
        if(!s) {
            commandT = INVALID;
            return;
        } else {
            name = s;
            message = command.substr(name.size() + 6); // 6 = 2 spaces + "send"
        }
    } else if(!strcmp(s, "who")) {
        commandT = WHO;
    } else if(!strcmp(s, "exit")) {
        commandT = EXIT;
    } else if(!strcmp(s, "name")) {
        commandT = NAME;
        s = strtok_r(NULL, " ", &saveptr);
        if(!s) {
            commandT = INVALID;
            return;
        } else
        {
            name = s;
        }
    } else {
        commandT = INVALID;
    }
}
