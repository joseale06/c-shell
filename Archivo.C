#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

typedef enum { OP_NONE, OP_SEMICOLON, OP_AND, OP_OR } OperatorType;

typedef struct {
    char **args;
    OperatorType next_op;
} Command;

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
    int i = 0;
    while (i < cmd_count) {
        if (i > 0) {
            OperatorType prev_op = cmd_list[i-1].next_op;
            if (prev_op == OP_AND && last_status != 0) { i++; continue; }
            if (prev_op == OP_OR && last_status == 0) { i++; continue; }
        }
        last_status = launch_process(cmd_list[i].args);
        i++;
    }
    return last_status;
}

char **parse_args(char *line) {
    char **args = (char **)malloc(64 * sizeof(char *));
    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; 
    return args;
}
int main() {
    char linea[2048];
    signal(SIGINT, SIG_IGN);

    while (1) {
        printf("ucvsh> ");
        if (!fgets(linea, sizeof(linea), stdin)) break;
        linea[strcspn(linea, "\n")] = 0;
        if (strlen(linea) == 0) continue;

        if (strcmp(linea, "exit") == 0) exit(0);

        Command cmd;
        cmd.args = parse_args(linea);
        cmd.next_op = OP_NONE;

        execute_command_list(&cmd, 1);
        
        free(cmd.args);
    }
    return 0;
}