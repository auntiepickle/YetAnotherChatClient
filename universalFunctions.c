//
//  universalFunctions.c
//  
//
//  Created by Chris Gross on 12/8/11.
//  Copyright 2011 Case Western Reserve University All rights reserved.
//

void error(const char *msg) //prints errors
{
    perror(msg);
    exit(1);
}

int exitCall(char *str) //checks for exit
{
    if(!strcmp(str, EXIT_CALL) || ! strcmp(str, TIMEOUT_CALL))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int chop(char *str) //chops off the new line
{
    int len = strlen(str);
    
    if( str[len-1] == '\n' )
    {
        str[len-1] = 0;
    }
    else if(len == 0)
    {
        return 0;
    }
    
    return 1;
}

void zeroBuffer(char *buffer)
{    
    memset(buffer, 0, MAX_MESSAGE_SIZE); //zero out the buffer
}

void addCommand(char *buffer, int command)
{    
    memmove(buffer, (buffer - 2), MAX_MESSAGE_SIZE);
    buffer[COMMAND_LOCATION] = command;
}

void clearScreen()
{
    if (system("cls")) 
        system("clear");
}

void grabContents(char *buffer) //grabs contents of the buffer ie the message minus the command
{
    int x = 0;
    char character = buffer[COMMAND_LOCATION + 1];
    
    while (character != '\0')
    {
        character = buffer [x + COMMAND_LOCATION + 1];
        buffer[x] = character;
        x++;
    }
    buffer[x] = 0;
    buffer[x - 1] = 0; 
}