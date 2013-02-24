//
//  client.c
//  
//
//  Created by Chris Gross on 12/8/11.
//  Copyright 2011 Case Western Reserve University All rights reserved.
//

#include "constants.h"

int     sockfdServer, initSock, portno, n, clilen, sockfd, port;
int     pipes[NUM_PIPES];
int     pidServer;
struct  sockaddr_in serv_addr, chat_addr;
struct  hostent *server;
char    userName[USERNAME_SIZE];
char    yourName[USERNAME_SIZE];

void readUserInput(char *buffer)
{    
    fgets(buffer, MAX_MESSAGE_SIZE, stdin);
}

void handleMessage (int sock)
{
    int n;
    char buffer[MAX_MESSAGE_SIZE];
    
    zeroBuffer(buffer);
    n = read(sock,buffer,MAX_MESSAGE_SIZE);
    
    if (n < 0) 
    {
        error("ERROR reading from socket");
    }
    else if (buffer[COMMAND_LOCATION] == CHAT) //checks for chatting
    {        
        clearScreen();        
        grabContents(buffer);
        chop(buffer);
        printf("%s\n",buffer);
        zeroBuffer(buffer);
        addCommand(buffer, CHAT);
        printf("Please enter a command: \n");
        
        while(buffer[COMMAND_LOCATION] == CHAT)
        {                
            zeroBuffer(buffer);
            printf("");
            
            read(sock,buffer,MAX_MESSAGE_SIZE);
            printf("");
            
            if(buffer[COMMAND_LOCATION] == CHAT)
            {
                grabContents(buffer);
                
                if(strcmp(buffer, END_CHAT_CALL) == 0)
                {
                    printf("end chat received\n");
                    break;
                }
                
                printf("%s\n",buffer);
                addCommand(buffer, CHAT);
                
                printf("\rPlease enter a command: \n");
            }
        } 
        
        printf("Please enter a command: ");
    }
    else
    {
        clearScreen();
        printf("%s",buffer);
        printf("Please enter a command: ");
    }
}

void setUpServer(int port) //sets up the message server for the client
{
    close(pipes[0]); //close pipe so we can write
    
    int sockfd, newsockfd, pid;
    unsigned short portno;
    socklen_t clilen;
    struct sockaddr_in serv2_addr, cli_addr;
    
    if (port == 0)
    {
        portno = DEFAULT_PORT;
    }
    else
    {
        portno = port;
    }
    
    int bindStatus = -1;
    
    while (bindStatus < 0 && portno < MAX_PORT_SIZE)
    {
        portno++;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        if (sockfd < 0) 
        {
            error("ERROR opening socket");
        }
        
        bzero((char *) &serv2_addr, sizeof(serv2_addr));
        serv2_addr.sin_family = AF_INET;
        serv2_addr.sin_addr.s_addr = INADDR_ANY;
        serv2_addr.sin_port = htons(portno);
        bindStatus = bind(sockfd, (struct sockaddr *) &serv2_addr, sizeof(serv2_addr));
        serv2_addr.sin_port = htons(portno);              
    }
    
    if (bindStatus < 0)
    {
        error("ERROR on binding.. PORT OUT OF RANGE\n");
    }
    
    write(pipes[1], &portno, sizeof(portno));// send out the port
    
    listen(sockfd, LISTENING_QUEUE);
    clilen = sizeof(cli_addr);
    
    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        if (newsockfd < 0) 
        {
            error("ERROR on accept");
        }
        //fork off a process to handle this request
        pid = fork();
        
        if (pid < 0)
        {
            error("ERROR on fork");
        }
        if (pid == 0)  
        {
            close(sockfd);
            handleMessage(newsockfd);
            exit(0);
        }
        else 
        {
            close(newsockfd);
        }
    }
    
    close(sockfd);
    exit(0);
}

void chatConnect(char *buffer, struct sockaddr_in user) //chat processing
{
    int clientfdServer;
    struct hostent *chatServer;
    struct sockaddr_in serv_addrChat;
    clientfdServer = socket(AF_INET, SOCK_STREAM, 0); //TCP
    
    if (clientfdServer < 0) 
    {
        error("ERROR opening socket");
    }
    
    chatServer = gethostbyaddr((char*)&user.sin_addr.s_addr, 4, AF_INET); //chatServer hostname or IP
    
    if (chatServer == NULL) //check if we have a connection
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addrChat, sizeof(serv_addrChat)); //zero out the field
    serv_addrChat.sin_family = AF_INET;
    bcopy((char *)chatServer->h_addr, (char *)&serv_addrChat.sin_addr.s_addr, chatServer->h_length);
    serv_addrChat.sin_port = htons(user.sin_port); //port conversion
    
    if (connect(clientfdServer,(struct sockaddr *) &serv_addrChat,sizeof(serv_addrChat)) < 0) //try to connect
    {
        error("ERROR connecting");
    }
    
    char sendBuffer [MAX_MESSAGE_SIZE];
    
    sprintf(sendBuffer, "%s says: %s", userName, buffer);
    addCommand(sendBuffer, CHAT); //add chat command
    
    write(clientfdServer, sendBuffer, MAX_MESSAGE_SIZE);
    
    printf("Now chatting\n"); //everything went as planned
    
    zeroBuffer(buffer);
    readUserInput(buffer);
    
    while (strcmp(buffer, END_CHAT_CALL) != 0)
    {
        addCommand(buffer, CHAT);
        write(clientfdServer, buffer, MAX_MESSAGE_SIZE);
        zeroBuffer(buffer);
        readUserInput(buffer);
        chop(buffer);
    }
    
    write(clientfdServer, buffer, MAX_MESSAGE_SIZE);
    printf("Chat ended\n");
}

void forkConnect(char *buffer, struct sockaddr_in user) //forks a connection
{
    int clientfdServer;
    struct hostent *chatServer;
    struct sockaddr_in serv_addrChat;
    clientfdServer = socket(AF_INET, SOCK_STREAM, 0); //TCP
    
    if (clientfdServer < 0) 
    {
        error("ERROR opening socket");
    }
    
    chatServer = gethostbyaddr((char*)&user.sin_addr.s_addr, 4, AF_INET); //chatServer hostname or IP
    
    if (chatServer == NULL) //check if we have a connection
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addrChat, sizeof(serv_addrChat)); //zero out the field
    serv_addrChat.sin_family = AF_INET;
    bcopy((char *)chatServer->h_addr, (char *)&serv_addrChat.sin_addr.s_addr, chatServer->h_length);
    serv_addrChat.sin_port = htons(user.sin_port); //port conversion
    
    if (connect(clientfdServer,(struct sockaddr *) &serv_addrChat,sizeof(serv_addrChat)) < 0) //try to connect
    {
        error("ERROR connecting");
    }
    
    char sendBuffer [MAX_MESSAGE_SIZE];
    
    sprintf(sendBuffer, "%s says: %s", userName, buffer);
    write(clientfdServer, sendBuffer, MAX_MESSAGE_SIZE);
    
    clearScreen();
    printf("\nMessage Sent: %s\n", buffer); //everything went as planned    
    printf("Please enter a command: ");
}

void connectToServer(int argc, char *argv[])
{    
    if (argc < MIN_CLIENT_ARG) //check that the inputs are of the right size
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    
    portno = atoi(argv[PORT_ARG_CLIENT]); //grab port number from input
    
    sockfdServer = socket(AF_INET, SOCK_STREAM, 0); //TCP
    initSock = sockfdServer;
    
    if (sockfdServer < 0) 
    {
        error("ERROR opening socket");
    }
    
    server = gethostbyname(argv[HOST_NAME_ARG]); //server hostname
    
    if (server == NULL) //check if we have a connection
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr)); //zero out the field
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); //port conversion
    
    if (connect(sockfdServer,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //try to connect
    {
        error("ERROR connecting");
    }
    
    printf("Connection to server established\n"); //everything went as planned
}

void writeMessage(char *buffer)
{
    if(sockfdServer != GOOD_CONNECT)
        sockfdServer = GOOD_CONNECT;
    int n = write(sockfdServer, buffer, MAX_MESSAGE_SIZE);
    
    if (n < 0) 
    {
        error("ERROR writing to socket");
    }
}

void readMessage(char *buffer)
{    
    int n = read(sockfdServer, buffer, MAX_MESSAGE_SIZE);
    
    if (n < 0)
    {
        error("ERROR reading from socket");
    }
}

void disconnectCall(char *buffer) //handles disconnection call
{
    zeroBuffer(buffer);
    addCommand(buffer, DISCONNECT);
}

void usernameCall(char *buffer) //handles username call
{
    printf("Enter a new username:\n");
    {
        zeroBuffer(buffer);
        readUserInput(buffer);
    }
    sprintf(userName, "%s", buffer);
    addCommand(buffer, USERNAME);
    writeMessage(buffer);
    readMessage(buffer);
    if(buffer[COMMAND_LOCATION] == ASCII_TAKEN)
    {
        grabContents(buffer);
        printf("%s", buffer);
    }
    else
    {
        zeroBuffer(yourName);
        sprintf(yourName, "%s", userName);
        printf("%s", buffer);
    }
}

void listUserCall(char *buffer) //handles list users calls
{
    zeroBuffer(buffer);
    addCommand(buffer, LIST_USERS);
}

void portCall(char *buffer) //handles change of port call
{
    printf("Enter a new port number to be reached on:\n");
    
    {
        zeroBuffer(buffer);
        readUserInput(buffer);
    }
    
    addCommand(buffer, PORT);    
}

int checkReturnOnUser(int result) //checks the reun on user
{   
    if (result == VALID_USER)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void helpCall(char *buffer)
{
    zeroBuffer(buffer);
    clearScreen();
    printf("********************COMMANDS********************\n");
    printf("*          message:     messages a user        *\n");
    printf("*          port:        changes YOUR chat port *\n");
    printf("*          username:    changes YOUR username  *\n");
    printf("*          disconnect:  disconnects and exits  *\n");
    printf("*          chat:        disables commands and  *\n");
    printf("*                       just sends messages    *\n");
    printf("*          list users:  lists all online users *\n");    
    printf("*          <end chat>:  ends chat and enables  *\n");
    printf("*                       commands again         *\n"); 
    printf("*          help:        displays commands      *\n");
    printf("************************************************\n");
}

void chatCall(char *buffer) //handles chatting
{
    printf("Which user would you like to reach:\n");
    
    zeroBuffer(buffer);
    readUserInput(buffer);    
    chop(buffer);
    if(strcmp(buffer, yourName) == 0)
    {
        printf("You can't talk to yourself\n");
        return;
    }
    char chattingWith[USERNAME_SIZE];
    sprintf(chattingWith, "%s", buffer);
    
    addCommand(buffer, MESSAGE);
    writeMessage(buffer); //write out attempted user call
    zeroBuffer(buffer);
    
    int result;
    read(sockfdServer, &result, sizeof(result));
    
    if (checkReturnOnUser(result))
    {
        printf("Now chatting with %s\n", chattingWith);
        struct sockaddr_in user;            
        read(sockfdServer, &user, sizeof(user));
        zeroBuffer(buffer);
        readUserInput(buffer);
        chatConnect(buffer, user);
    }
    else 
    {
        printf("ERROR: Unable to find user\n");
    }
    
}

void messageCall(char *buffer) //handles message call
{    
    printf("Which user would you like to reach:\n");
    
    zeroBuffer(buffer);
    readUserInput(buffer);   
    chop(buffer);
    if(strcmp(buffer, yourName) == 0)
    {
        printf("You can't talk to yourself\n");
        return;
    }
    addCommand(buffer, MESSAGE);
    writeMessage(buffer); //write out attempted user call
    zeroBuffer(buffer);
    
    int result;
    read(sockfdServer, &result, sizeof(result));
    
    if (checkReturnOnUser(result))
    {
        printf("Please enter your message:\n");
        struct sockaddr_in user;            
        read(sockfdServer, &user, sizeof(user));
        zeroBuffer(buffer);
        readUserInput(buffer);
        
        int pid = fork(); //forks off a process to send the message
        if (pid == 0)
        {
            forkConnect(buffer, user);
            exit(0);
        }
    }
    else 
    {
        printf("ERROR: Unable to find user\n");
    }
}

int commandInput(char *buffer) //main input
{
    printf("Please enter a command: ");
    
    zeroBuffer(buffer);
    readUserInput(buffer);
    chop(buffer);
    
    if(strcmp(buffer, EXIT_CALL) == 0) 
    {
        disconnectCall(buffer);
        return 0;
    }
    else if(strcmp(buffer, USERNAME_TEXT_CALL) == 0)
    {
        usernameCall(buffer);
        commandInput(buffer);
    }
    else if(strcmp(buffer, LIST_TEXT_CALL) == 0)
    {
        listUserCall(buffer);
    }
    else if(strcmp(buffer, PORT_CALL) == 0)
    {
        portCall(buffer);
    }
    else if(strcmp(buffer, MESSAGE_CALL) == 0)
    {
        messageCall(buffer);
        commandInput(buffer);
    }
    else if(strcmp(buffer, CHAT_CALL) == 0)
    {
        chatCall(buffer);
        commandInput(buffer);
    }
    else if(strcmp(buffer, HELP_CALL) == 0)
    {
        helpCall(buffer);
        commandInput(buffer);
    }
    else
    {
        printf("I am sorry, I don't recognize that command..\n");
        commandInput(buffer);
    }
    return 1;
}

int checkValidUserName(char *buffer) //checks usernames are withing ASCII region
{
    int x = 0;
    char currentChar = -1;
    while (x < USERNAME_SIZE)
    {
        currentChar = buffer[x];
        if(currentChar == 0)
        {
            break;
        }
        else if(currentChar < ASCII_MIN || currentChar > ASCII_MAX)
        {
            return 0;
        }
        else
        {
            x++;
        }
    }
    return 1;
}

void setUpLoop()
{
    int conditionChecker = -1;
    char buffer[MAX_MESSAGE_SIZE];
    char userNameBuff[USERNAME_SIZE];
    
    //Let the server know which port we will be taking messages on
    zeroBuffer(buffer);
    sprintf(buffer, "%d", portno);
    addCommand(buffer, PORT);
    writeMessage(buffer);
    readMessage(buffer);
    printf("%s", buffer);
    
    //Initialize a username.. must have a username
    zeroBuffer(buffer);
    
    while (conditionChecker != 1)
    {
        printf("Please input a valid username:\n");
        zeroBuffer(buffer);
        readUserInput(buffer);
        chop(buffer);
        conditionChecker = checkValidUserName(buffer);
    }
    //send out username
    sprintf(userName, "%s", buffer);
    zeroBuffer(yourName);
    sprintf(yourName, "%s", buffer);
    addCommand(buffer, USERNAME);
    printf("wrote to server\n");
    writeMessage(buffer);
    printf("reading from server\n");
    readMessage(buffer);
    
    
    printf("\n\nvalid\n\n");
    while (buffer[COMMAND_LOCATION] == ASCII_TAKEN) //if name is all ready taken get one that isnt
    {           
        conditionChecker = 0;
        grabContents(buffer);
        printf("%s", buffer);
        zeroBuffer(buffer);
        
        while (conditionChecker != 1)
        {
            printf("Please input a valid username:\n");
            zeroBuffer(buffer);
            readUserInput(buffer);
            chop(buffer);
            conditionChecker = checkValidUserName(buffer);
        }
        
        sprintf(userName, "%s", buffer);
        zeroBuffer(yourName);
        sprintf(yourName, "%s", buffer);
        addCommand(buffer, USERNAME);
        writeMessage(buffer);   
        readMessage(buffer);
    }
    
    printf("%s", buffer);
    zeroBuffer(buffer);
    
}
void mainLoop()
{    
    char buffer[MAX_MESSAGE_SIZE];
    int exitStatus = 1;
    
     pipe(pipes);
    
    pidServer  = fork(); //sets up message server
     if (pidServer == 0)
    {
        setUpServer(0);
         exit(0);
    }
    else
    {
            close(pipes[1]); //close the write side
            read(pipes[0], &portno, sizeof(portno));// get the new port
    }
    setUpLoop();
    clearScreen();
    
    while(exitStatus)
    {
        exitStatus = commandInput(buffer);
        writeMessage(buffer); // send buffer contents
        if(buffer[COMMAND_LOCATION] == DISCONNECT)
        {
            break;
        }
        zeroBuffer(buffer);        
        
        readMessage(buffer);
        printf("%s", buffer);
    }
    printf("disconnected..\n");
}


int main(int argc, char *argv[]) //main function
{
    printf("connecting to server\n");
    connectToServer(argc, argv);
    
    mainLoop();
    close(sockfdServer);
    printf("SIGKILL: %d\n", SIGKILL);
    kill(pidServer, SIGKILL); //kills children
    return 0;
}
