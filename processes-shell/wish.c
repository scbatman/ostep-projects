#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

void exec_ls_command(char * ls_arguments[]){
    char error_message[30] = "An error has occurred\n";
    execvp("ls", ls_arguments);
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void execute_command(char ** arguments, int starting_int, int i){
    char * exit_command = "exit";
    char * ls_command = "ls";
    char * cd_command = "cd";
    char * path_command = "path";
    char error_message[30] = "An error has occurred\n";

    bool fileOutput = false;
    int fd;
    if (i - starting_int >= 3 && strcmp(arguments[i-2], ">") == 0){
        fileOutput = true;
        fd = open(arguments[i-1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        arguments[i - 2] = '\0';
        arguments[i - 1] = '\0';
    }
    if (strcmp(arguments[starting_int], exit_command) == 0){
        exit(0);
    }
    else if (strcmp(arguments[starting_int], ls_command) == 0){
        char * ls_args[i - starting_int + 1];
        int index = 0;
        for (int z = starting_int; z < i; z++){
            ls_args[index++] = arguments[z];
        }
        ls_args[index] = '\0';
        if (fileOutput){
            pid_t p;
            p = fork();
            if (p == 0){
                exec_ls_command(ls_args);
            }
            else if (p > 0){
                wait(NULL);
                close(fd);
                exit(0);
            }
            else{
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        else{
            execvp("ls", ls_args);
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    else if (strcmp(arguments[starting_int], cd_command) == 0){
        if (i > 2){
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else if (i < 2){
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else{
            chdir(arguments[starting_int + 1]);
        }
    }
    else if (strcmp(arguments[starting_int], path_command) == 0){
        char path_string[100] = "";
        for (int j = 1; j < i; j++){
            strcat(path_string, arguments[j]);
            if (j < i - 1) {
                strcat(path_string, " ");
            }
        }
        setenv("PATH", path_string, 1);
    }
    exit(0);
}

int main (int argc, char* argv[]){

    char * exit_command = "exit";
    char error_message[30] = "An error has occurred\n";
    char * cd_command = "cd";
    char * buffer = NULL;
    size_t line_length;
    
    if (argc > 1){
        FILE * fp;
        fp = fopen(argv[1], "r");

        while (getline(&buffer, &line_length, fp) != -1){
            char ** arguments = (char**)malloc(100 * sizeof(char *));
            char * argument = NULL;
            int k = 0;

            argument = strtok(buffer, " ");
            while (argument != NULL){
                arguments[k++] = argument;
                argument = strtok(NULL, " ");
            }
            pid_t * processes = (pid_t*)malloc(100 * sizeof(pid_t));
            int * argument_indices = (int*)malloc(100 * sizeof(int));
            argument_indices[0] = -1;
            int num = 0;

            int last_command = -1;
            for (int j = 0; j < k;j++){
                if (strcmp(arguments[j], "&") == 0){
                    pid_t p = fork();
                    processes[num++] = p;
                    argument_indices[num] = j;
                    if (p == 0){
                        execute_command(arguments, argument_indices[num - 1] + 1, j);
                    }
                    else{
                        continue;
                    }
                }
            }
            for (int a = 0; a < num; a++){
                int status;
                waitpid(processes[a], &status, 0);
            }
            arguments[k - 1][strlen(arguments[k-1]) - 1] = '\0';
            if (strcmp(arguments[argument_indices[num] + 1], exit_command) == 0){
                free(arguments);
                free(processes);
                free(argument_indices);
                free(buffer);
                exit(0);
            }
            else if (strcmp(arguments[argument_indices[num] + 1], cd_command) == 0){
                if (k != 2){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                else{
                    chdir(arguments[argument_indices[num] + 2]);
                }
            }
            else{
                pid_t p = fork();
                if (p == 0){
                    execute_command(arguments, argument_indices[num] + 1, k);
                }
                else if (p > 0){
                    wait(NULL);
                }
                buffer = NULL;
                free(arguments);
                free(processes);
                free(argument_indices);
            }
        }
        free(buffer);
    }
    else{
        while (1){
            printf("wish>");
            getline(&buffer, &line_length, stdin);

            char ** arguments = (char**)malloc(100 * sizeof(char *));
            char * argument = NULL;
            int k = 0;

            argument = strtok(buffer, " ");
            while (argument != NULL){
                arguments[k++] = argument;
                argument = strtok(NULL, " ");
            }
            pid_t * processes = (pid_t*)malloc(100 * sizeof(pid_t));
            int * argument_indices = (int*)malloc(100 * sizeof(int));
            argument_indices[0] = -1;
            int num = 0;

            int last_command = -1;
            for (int j = 0; j < k;j++){
                if (strcmp(arguments[j], "&") == 0){
                    pid_t p = fork();
                    processes[num++] = p;
                    argument_indices[num] = j;
                    if (p == 0){
                        execute_command(arguments, argument_indices[num - 1] + 1, j);
                    }
                    else{
                        continue;
                    }
                }
            }
            for (int a = 0; a < num; a++){
                int status;
                waitpid(processes[a], &status, 0);
            }
            arguments[k - 1][strlen(arguments[k-1]) - 1] = '\0';
            if (strcmp(arguments[argument_indices[num] + 1], exit_command) == 0){
                free(arguments);
                free(processes);
                free(argument_indices);
                free(buffer);
                exit(0);
            }
            else if (strcmp(arguments[argument_indices[num] + 1], cd_command) == 0){
                if (k != 2){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                else{
                    chdir(arguments[argument_indices[num] + 2]);
                }
            }
            else{
                pid_t p = fork();
                if (p == 0){
                    execute_command(arguments, argument_indices[num] + 1, k);
                }
                else if (p > 0){
                    wait(NULL);
                }
                buffer = NULL;
                free(arguments);
                free(processes);
                free(argument_indices);
            }
        }
    free(buffer);
    }
}