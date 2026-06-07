#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/parser.h"
#include "../../include/core/executor.h"
#define MAX_BUFFER 1024
int main() {
    char input[MAX_BUFFER];

    while (1) {
        printf("\033[1;33mucvsh >\033[0m ");
        fflush(stdout);

        if (fgets(input, MAX_BUFFER, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        int cmd_count = 0;
        CommandStruct **cmd_list = parseInput(input, &cmd_count);

        if (cmd_list == NULL) continue;

        execute_command_list(cmd_list, cmd_count);
        freeCommandList(cmd_list, cmd_count);
    }

    return 0;
}