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
    *cmd_count = 1;
    Command *list = malloc(sizeof(Command));
    list[0].args = malloc(64 * sizeof(char *));
    
    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        list[0].args[i++] = token;
        token = strtok(NULL, " ");
    }
    list[0].args[i] = NULL;
    list[0].next_op = OP_NONE;
    return list;
}