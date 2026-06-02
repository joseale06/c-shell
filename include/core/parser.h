#ifndef PARSER_H  
#define PARSER_H

typedef struct CommandStruct {
    char *command;    
    char **cmd_args;  
    int num_args;   
    int background;      
} CommandStruct;

CommandStruct* parseInput(char *input);
void print_command_debug(CommandStruct *cmd);
void freeCommandStruct(CommandStruct *cmd);

#endif