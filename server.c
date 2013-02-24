//
//  server.c
//  
//
//  Created by Chris Gross on 12/8/11.
//  Copyright 2011 Case Western Reserve University All rights reserved.
//
#include "constants.h"
#include <fcntl.h>

struct user
{
    char username[USERNAME_SIZE];
    struct sockaddr_in address;
};

int     highsock;

int     sockfd, newsockfd, portno;
int     reuse_addr = 1;
socklen_t clilen;
struct  sockaddr_in serv_addr, cli_addr;

fd_set  readfds;
struct  timeval timeout;

int     fd[MAX_USERS];
int     nbytes;

struct user users[MAX_USERS];
int numUsers;

void handleConnection(int); 
void handleData(int);

void setnonblocking(int sock) //Sets the connection as non blocking
{
    int opts;
    
    opts = fcntl(sock,F_GETFL);
    
    if (opts < 0) 
    {
        error("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    
    opts = (opts | O_NONBLOCK);
    
    if (fcntl(sock,F_SETFL,opts) < 0) 
    {
        error("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

void setUpServer(int argc, char *argv[]) //Sets up the server
{
    timeout.tv_sec  = TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_MS;
    
    
    if (argc < MIN_ARGS) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    //set up the socket. we are using TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)  //make sure we can open the socket
    {
        error("ERROR opening socket");
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)); //allow reuse of address
    
    setnonblocking(sockfd); //nonblocking
    
    //port stuff
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    portno = atoi(argv[PORT_ARG_SERVER]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //check it we can bind it
    {
        error("ERROR on binding");
    }
    
    listen(sockfd, LISTENING_QUEUE); //set up a listening queue
    
    clilen = sizeof(cli_addr); //set the length of the address
}

int setDescriptors() //set up the fields to check for connections
{
    int x = 0;
    int returnVal = sockfd;
    
    for (x = 0; x < MAX_USERS ; x++)
    {
        if (fd[x] != 0)
        {
            FD_SET(fd[x], &readfds);
        }
        
        if (fd[x] > returnVal)
        {
            returnVal = fd[x];
        }
    }
    
    return returnVal; //returns highest valued field
}

void clearUsers()//Clears out all the users
{    
    int x = 0;
    
    for (x = 0; x < MAX_USERS; x++)
    {
        memset(users[x].username, 0, USERNAME_SIZE);
    }
}

void handle_new_connection() 
{
	int listnum;	    //Current item in fd
	int connection;     // Socket file descriptor for incoming connections 
    
	connection = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
	if (connection < 0) 
    {
		perror("accept");
		exit(EXIT_FAILURE);
	}
    
	setnonblocking(connection);
    
	for (listnum = 0; (listnum < MAX_USERS) && (connection != -1); listnum ++)
		if (fd[listnum] == 0) 
        {
			printf("\nConnection accepted:   FD=%d; Slot=%d\n", connection, listnum);
			fd[listnum] = connection;
			connection = -1;
		}
    
	if (connection != -1) 
    {
		// No room left in the queue!
		printf("\nNo room left for new client.\n");
    }
    else
    {
        struct user newUser;
        newUser.address.sin_addr.s_addr = cli_addr.sin_addr.s_addr; //store the ip
        printf("USER IP: %u at %d\n", newUser.address.sin_addr.s_addr, listnum - 1);
        users[fd[listnum - 1]] = newUser;
    }
}

void readSocks() //check all the sockets
{
	int x;	    
	
    if (FD_ISSET(sockfd, &readfds))
    {
		handle_new_connection();
    }
	
	
	// Run through our sockets and check to see if anything happened with them
	for (x = 0; x < MAX_USERS; x++) 
    {
		if (FD_ISSET(fd[x], &readfds))
        {
            printf("handling user: %d\n", x);
			handleData(x);
        }
	} 
}


void connectionWaiting()
{
    int new_conns;
    int max_field;
    
    FD_ZERO(&readfds); //set up the list
    FD_SET(sockfd, &readfds); //listen for new connections
    
    max_field = setDescriptors();
    new_conns = select(max_field + 1/*+1 is defined max size plus one see man*/,
                       &readfds, (fd_set *) 0, (fd_set *) 0, &timeout); //now we wait for timeout length
    
    if (new_conns < 0) 
    {
        error("ERROR on connections");
        exit(EXIT_FAILURE);
    }
    if (new_conns == 0) 
    {
        //nothing new
        //printf(".");
        fflush(stdout);
    } 
    else
        readSocks();
}

int main(int argc, char *argv[])
{
    setUpServer(argc, argv); //start the server
    
    memset((char *) &fd, 0, sizeof(fd)); //clear out the fd
    clearUsers();   //clear out the user list
    
    while (1) //infinite loop
    {
        connectionWaiting();
    } 
    
    close(sockfd);
    
    return 0; //we never get here
}

void usernameHandler(char *buffer, int sock) //handles username requests
{
    grabContents(buffer); //get the actual message minus command
    chop(buffer); //chop off anything extraneous
    
    int x = 0;
    
    for (x = 0; x < MAX_USERS; x++) //check username with all users and return bad if not found
    {
        if (strcmp(users[x].username, buffer) == 0)
        {
            char tempBuffer[MAX_MESSAGE_SIZE];
            sprintf(tempBuffer, " %dError: username '%s' all ready taken\n", NAME_TAKEN, buffer);
            writeMessageDefined(sock, tempBuffer);
            return;
        }
    }
    
    //if good return a good call and establish the username
    sprintf(users[sock].username, "%s", buffer);
    printf("\nUsername established: %s\n", users[sock].username);  
    
    zeroBuffer(buffer);
    sprintf(buffer, "Server: Username established '%s'\n", users[sock].username);
    
    writeMessageDefined(sock, buffer);
}

void disconnectHandler(char *buffer, int sock)
{
    memset(users[sock].username, 0, USERNAME_SIZE); //clear out that users name
    zeroBuffer(buffer);
}

void listHandler(char *buffer, int sock)
{
    printf("\nlisting users\n");
    int loc = 0;
    int x = 0;
    zeroBuffer(buffer);
    
    for (x = 0; x < MAX_USERS; x++) //grab all the users and print their names
    {
        if (users[x].username[0] != 0)
        {
            int len = strlen(users[x].username);
            int y = 0;
            
            for (y = 0; y < len; y++)
            {
                buffer[loc] =  users[x].username[y];
                loc++;
            }
            buffer[loc] =  '\n';
            loc++;
        }
    }
    //send the message
    printf("%s",buffer);
    writeMessageDefined(sock, buffer);
}

void portHandler(char *buffer, int sock) //saves port to user for chatting
{
    grabContents(buffer);
    
    users[sock].address.sin_port = atoi(buffer);
    
    printf("Port established for user '%s': %u\n", users[sock].username, users[sock].address.sin_port); 
    
    zeroBuffer(buffer);
    
    sprintf(buffer, "Port established: %u\n", users[sock].address.sin_port);
    writeMessageDefined(sock, buffer);
}

void messageUserHandler(char *buffer, int sock) //retrieves the requested users info if it exists
{
    int x = 0;
    grabContents(buffer);
    chop(buffer);
    
    for (x = 0; x < MAX_USERS; x++)
    {
        if (strcmp(users[x].username, buffer) == 0)
        {
            printf("found user: '%s' on port: '%u'\n", users[x].username, users[x].address.sin_port);
            int result = VALID_USER;

            write(sock, &result, sizeof(result));
            write(sock, &users[x].address, sizeof(users[x].address));
            return;
        }
    }
    writeMessage(sock);
}

void readMessage(char *buffer, int sock) //parse the commands read
{
    int readStatus;
    zeroBuffer(buffer);
    readStatus = read(sock, buffer, MAX_MESSAGE_SIZE);
        
    printf("command read: %d\n", (int)buffer[COMMAND_LOCATION]);
    
    switch ((int)buffer[COMMAND_LOCATION]) 
    {
        case PORT:
            portHandler(buffer, sock);
            break;
            
        case USERNAME:
            usernameHandler(buffer, sock);
            break;
            
        case DISCONNECT:
            disconnectHandler(buffer, sock);
            break;
            
        case LIST_USERS:
            listHandler(buffer, sock);
            break;
            
        case MESSAGE:
            messageUserHandler(buffer, sock);
            break;
            
        default:
            printf("\nCommand not supported: (%d)\n", buffer[COMMAND_LOCATION]);
            writeMessageDefined(sock, buffer);
            break;
    }
    
    if (readStatus < 0)
    {
        error("ERROR reading from socket");
    }
    else
    {
        if(!chop(buffer)) //remove any unwanted characters
        {            
            memset(buffer, 0, MAX_MESSAGE_SIZE);
            buffer = TIMEOUT_CALL;
        }
    }
}

int writeMessageDefined(int sock, char *message)
{
    return write(sock, message, strlen(message));
}

int writeMessage(int sock) //default sent message deprecated don't use this
{
    return write(sock, "I got your message", 18);
}

void handleData (int sock) //handles an new data
{
    char buffer[MAX_MESSAGE_SIZE];     // Buffer for socket reads
    printf("reading a new message\n");
    readMessage(buffer, fd[sock]);  //reads a message
    
    int exitStatus = exitCall(buffer); //checks exit
    
	if (!exitStatus) 
    {
		//Connection closed, close this end and free up entry in fd 
		printf("\nConnection closed: FD=%d;  Slot=%d\n", fd[sock], sock);
        writeMessageDefined(sock, buffer);
		close(fd[sock]);
		fd[sock] = 0;
	} 
}