#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

void exec_ls_command(char ** arguments){
    execvp("ls", arguments);
}

int main (int argc, char* argv[]){

    char * buffer = NULL;
    char * exit_command = "exit";
    char * ls_command = "ls";
    char * cd_command = "cd";
    char * path_command = "path";
    size_t line_length;

    while (1){
        printf("wish>");
        getline(&buffer, &line_length, stdin);

        char ** arguments = (char**)malloc(100 * sizeof(char *));
        char * argument = NULL;
        int i = 0;

        bool fileOutput = false;
        int fd;

        argument = strtok(buffer, " ");
        while (argument != NULL){
            arguments[i++] = argument;
            argument = strtok(NULL, " ");
        }

        arguments[i - 1][strlen(arguments[i-1]) - 1] = '\0';
        if (strcmp(arguments[i-2], ">") == 0){
            fileOutput = true;
            fd = open(arguments[i-1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            arguments[i - 2] = '\0';
            arguments[i - 1] = '\0';
        }

        if (strcmp(arguments[0], exit_command) == 0){
            exit(0);
        }
        else if (strcmp(arguments[0], ls_command) == 0){
            if (fileOutput){
                pid_t p;
                p = fork();
                if (p == 0){
                    exec_ls_command(arguments);
                }
                else if (p > 0){
                    wait(NULL);
                    close(fd);
                    exit(0);
                }
                else{
                    perror("fork failed");
                }
            }
            else{
                execvp("ls", arguments);
            }
        }
        else if (strcmp(arguments[0], cd_command) == 0){
            if (i > 2){
                printf("too many arguments");
            }
            else if (i < 2){
                printf("Needs one argument");
            }
            else{
                chdir(arguments[1]);
            }
        }
        else if (strcmp(arguments[0], path_command) == 0){
            char path_string[100] = "";
            for (int j = 1; j < i; j++){
                strcat(path_string, arguments[j]);
                if (j < i - 1) {
                    strcat(path_string, " ");
                }
            }
            setenv("PATH", path_string, 1);
        }

        free(arguments);
    }
    free(buffer);
}