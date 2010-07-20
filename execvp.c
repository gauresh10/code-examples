#include <unistd.h>
#include <stdio.h>

extern char **environ;

int main (int argc, char **argv)
{
    char *cmd = argv[1];
    char **args = argv + 2;
    int i;
    for (i = 0; i < argc; ++i) {
        fprintf(stdout, "%s ", argv[i]);
    }
    fflush(stdout);

    execvp(cmd, argv);
    return 0;
}

