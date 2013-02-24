//
//  constants.h
//  
//
//  Created by Chris Gross on 12/8/11.
//  Copyright 2011 Case Western Reserve University All rights reserved.
//

#ifndef _constants_h
#define _constants_h

#define LISTENING_QUEUE 10
#define TIMEOUT_SEC 1
#define TIMEOUT_MS 0
#define TIMEOUT_CALL ""
#define MAX_MESSAGE_SIZE 256
#define USERNAME_SIZE 80
#define MAX_USERS 10
#define MIN_ARGS 2
#define MIN_CLIENT_ARG 3
#define PORT_ARG_SERVER 1
#define PORT_ARG_CLIENT 2
#define HOST_NAME_ARG 1
#define NUM_PIPES 2
#define GOOD_CONNECT 3

#define USERNAME -1
#define DISCONNECT 1
#define LIST_USERS 2
#define MESSAGE 3
#define PORT 4
#define CHAT 5
#define BAD_CALL 100
#define DEFAULT_PORT  12345
#define MAX_PORT_SIZE 60000
//[A-Za-Z,_]
#define ASCII_MIN 65 //A
#define ASCII_MAX 122 //z

#define COMMAND_LOCATION 1
#define VALID_USER 1
#define NAME_TAKEN 9
#define ASCII_TAKEN 57

#define USERNAME_TEXT_CALL  "username"
#define LIST_TEXT_CALL      "list users"
#define EXIT_CALL           "disconnect"
#define MESSAGE_CALL        "message"
#define CHAT_CALL           "chat"
#define END_CHAT_CALL       "<end chat>"
#define PORT_CALL           "port"
#define HELP_CALL                "help"

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "universalFunctions.c"

#ifndef SIGKILL
#define SIGKILL 9
#endif
