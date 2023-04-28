#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/wait.h>



#define EXIT_EXECVP_FAILED 99
#define MAX_SIZE 150
#define MAX_STRING_SIZE 150
#define CALCULATE_ELAPSED_TIME(start_time, end_time) \
((end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0)

#define EXECUTION_SUCCESSFUL 0
#define EXECUTION_TIMED_OUT  1
#define EXECUTION_ERROR     -1

void compileCFile(char path[MAX_STRING_SIZE],char outPutFileName[MAX_STRING_SIZE]) {
    // Fork a child process
    pid_t pidfork = fork();
    if (pidfork == -1) {
        // Fork failed
        perror("fork failed");
        return;
    }
    else if (pidfork == 0) {
        // Child process
        // Prepare arguments for gcc command
        char* args[] = {"gcc","-o",outPutFileName,path, NULL};
        // Execute the gcc command with arguments
        execvp(args[0], args);
        // Print error message if execvp fails
        perror("execvp failed");
        // Exit child process with failure status
        exit(EXIT_EXECVP_FAILED);
    }
    // Parent process
    int status;
    // Wait for child process to complete
    wait(&status);
}



int executeFileWithFDs(char path[MAX_STRING_SIZE], int input_fd, int output_fd) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXECUTION_ERROR;
    } else if (pid == 0) {
        // Child process
        // Redirect input and output to files
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        // Execute the file
        execl(path, NULL);
        // If execl returns, it means an error occurred
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        wait(&status);
        struct timeval end_time;
        gettimeofday(&end_time, NULL);
        double elapsed_time = CALCULATE_ELAPSED_TIME(start_time, end_time);
        printf("time pass %f\n",elapsed_time);
        if (elapsed_time > 5.0) {
            return EXECUTION_TIMED_OUT;
        } else {
            return EXECUTION_SUCCESSFUL;
        }
    }
}


void executeFileWithFiles(char path[MAX_STRING_SIZE], char input_file[MAX_STRING_SIZE], char output_file[]) {
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("open input file");
        return;
    }
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        perror("open output file");
        close(input_fd);
        return;
    }
    int result = executeFileWithFDs(path, input_fd, output_fd);
    if (result == EXECUTION_ERROR) {
        perror("execution error");
    } else if (result == EXECUTION_TIMED_OUT) {
        printf("execution timed out\n");
    }
    close(input_fd);
    close(output_fd);
}


int main()
{
    compileCFile("files/students/Excellent_gets_100/great5.c.txt.txt.txt.c","outPutFileName");
    printf("complie\n");
    executeFileWithFiles("outPutFileName","files/io/input.txt","outputfiletest.txt");
    compileCFile("files/students/TIMEOUT_gets_20/Example.c","outPutFileName");
    printf("complie\n");
    executeFileWithFiles("outPutFileName","files/io/input.txt","outputfiletest.txt");
    unlink("outputfiletest.txt");
    unlink("outPutFileName");
    //todo to add a check fo the deletion
    return 0;
}

