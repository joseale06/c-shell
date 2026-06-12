#include <stdio.h>
#include <unistd.h>
#include "../../include/builtins/pwd.h"

int builtin_pwd(char **args) {
    (void)args;
    char cwd[1024];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("ucvsh: pwd");
        return 1;
    }
}