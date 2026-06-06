#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef enum { OP_NONE, OP_SEMICOLON, OP_AND, OP_OR } OperatorType;

typedef struct {
    char **args;
    OperatorType next_op;
} Command;

int launch_process(char **args);
int execute_command_list(Command *cmd_list, int cmd_count);
Command* parse_line(char *line, int *cmd_count);

int main() {
    char linea[1024];

    while (1) {
        printf("ucvsh> ");
        if (!fgets(linea, sizeof(linea), stdin)) break;
        
        linea[strcspn(linea, "\n")] = 0;

        if (strlen(linea) == 0) continue;
        if (strcmp(linea, "exit") == 0) break;

        int cmd_count = 0;
        Command *cmd_list = parse_line(linea, &cmd_count);
        
        if (cmd_list != NULL) {
            execute_command_list(cmd_list, cmd_count);
            
            for (int i = 0; i < cmd_count; i++) free(cmd_list[i].args);
            free(cmd_list);
        }
    }
    return 0;
}

int launch_process(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    } else if (pid < 0) return -1;

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int execute_command_list(Command *cmd_list, int cmd_count) {
    int last_status = 0;
    for (int i = 0; i < cmd_count; i++) {
        if (i > 0) {
            OperatorType prev_op = cmd_list[i-1].next_op;
            if (prev_op == OP_AND && last_status != 0) continue;
            if (prev_op == OP_OR && last_status == 0) continue;
        }
        last_status = launch_process(cmd_list[i].args);
    }
    return last_status;
}

Command* parse_line(char *line, int *cmd_count) {
    Command *list = (Command *)malloc(10 * sizeof(Command));
    *cmd_count = 0;
    char *saveptr1, *saveptr2;
    char *cmd_str = strtok_r(line, ";", &saveptr1);
    
    while (cmd_str != NULL) {
        list[*cmd_count].args = (char **)malloc(64 * sizeof(char *));
        list[*cmd_count].next_op = OP_SEMICOLON; 
        int i = 0;
        char *token = strtok_r(cmd_str, " ", &saveptr2);
        while (token != NULL) {
            list[*cmd_count].args[i++] = token;
            token = strtok_r(NULL, " ", &saveptr2);
        }
        list[*cmd_count].args[i] = NULL;
        (*cmd_count)++;
        cmd_str = strtok_r(NULL, ";", &saveptr1);
    }
    if (*cmd_count > 0) {
        list[*cmd_count - 1].next_op = OP_NONE;
    }
    
    return list;
}