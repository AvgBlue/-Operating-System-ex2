#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/wait.h>

#define EXIT_EXECVP_FAILED 99
#define MAX_SIZE 150
#define MAX_STRING_SIZE 150
#define CALCULATE_ELAPSED_TIME(start_time, end_time) \
    ((end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0)

#define EXECUTION_SUCCESSFUL 0
#define EXECUTION_TIMED_OUT 1
#define EXECUTION_ERROR -1

int executeCommand(char *args[], int input_fd, int output_fd, int error_fd, double *runtime)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Child process
        // Redirect input, output, and error to files
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        dup2(error_fd, STDERR_FILENO);
        // Execute the command
        execvp(args[0], args);
        // If execvp returns, it means an error occurred
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    // Parent process
    int status;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    wait(&status);
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    *runtime = CALCULATE_ELAPSED_TIME(start_time, end_time);
    return status >> 8;
}

int textCompare(char path1[MAX_STRING_SIZE], char path2[MAX_STRING_SIZE])
{
    int error_fd = open("compeionError", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (error_fd == -1)
    {
        perror("open");
        return -1;
    }
    double time;
    char *args1[] = {"gcc", "ex21.c", "-o", "ex21", NULL};
    ;
    int status = executeCommand(args1, STDIN_FILENO, STDOUT_FILENO, error_fd, &time);
    if(status == 1){
        return -1;
    }
    char *args2[] = {"./ex21", path1, path2, NULL};
    status = executeCommand(args2, STDIN_FILENO, STDOUT_FILENO, error_fd, &time);
    close(error_fd);
    if (unlink("compeionError") == -1)
    {
        // TODO to change the error
        perror("unlink failed");
    }
    return status;
}

int compileFile(char path[MAX_STRING_SIZE])
{
    int error_fd = open("compeionError", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (error_fd == -1)
    {
        perror("open");
        return -1;
    }
    char *args[] = {"gcc", path, NULL};
    double time;
    int status = executeCommand(args, STDIN_FILENO, STDOUT_FILENO, error_fd, &time);
    close(error_fd);
    if (unlink("compeionError") == -1)
    {
        // TODO to change the error
        perror("unlink failed");
    }
    // 0 is good 1 is bad
    return status;
}

int runFile(int input_fd, int output_fd)
{
    int error_fd = open("compeionError", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (error_fd == -1)
    {
        perror("open");
        return -1;
    }
    char *args[] = {"./a.out", NULL};
    double time;
    int status = executeCommand(args, input_fd, output_fd, error_fd, &time);
    close(error_fd);
    if (unlink("compeionError") == -1)
    {
        // TODO to change the error
        perror("unlink failed");
    }
    int timeStatus = 0;
    if (time > 5)
    {
        return EXECUTION_TIMED_OUT;
    }
    return EXECUTION_SUCCESSFUL;
}

void findCFile(const char *dir_path, char *c_file_path)
{
    DIR *dir;
    struct dirent *entry;
    const char *extension;
    size_t len;

    // Open the directory
    dir = opendir(dir_path);
    if (dir == NULL)
    {
        printf("Error opening directory\n");
        exit(EXIT_FAILURE);
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Get the length of the file name
        len = strlen(entry->d_name);

        // Skip entries that are not long enough to have an extension
        if (len <= 2)
        {
            continue;
        }

        // Get a pointer to the extension in the file name
        extension = entry->d_name + len - 2;

        // Check if the extension is ".c"
        if (strcmp(extension, ".c") == 0)
        {
            // If the file name ends with ".c", construct the path and copy it to the buffer
            if (strlen(dir_path) + len + 2 > MAX_STRING_SIZE)
            {
                printf("Path too long\n");
                exit(EXIT_FAILURE);
            }
            strcpy(c_file_path, dir_path);
            strcat(c_file_path, "/");
            strcat(c_file_path, entry->d_name);
            break;
        }
    }

    // Close the directory
    closedir(dir);
}

int grade(char path[MAX_STRING_SIZE], int input_fd, char cOutputPath[MAX_STRING_SIZE])
{
    int returnValue = 0;
    // TODO to set the file name to defined
    int output_fd = open("output", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1)
    {
        perror("open");
        return -1;
    }

    do
    {
        char cfile[MAX_STRING_SIZE] = {0};
        // look for c file
        findCFile(path, cfile);
        if (strcmp(cfile, "") == 0)
        {
            // grade is 0 NO_C_FILE
            returnValue = 0;
            break;
        }
        int compileStatus = compileFile(cfile);
        if (compileFile == 1)
        {
            // grade is 10 COMPILATION_ERROR
            // TODO set a define
            returnValue = 10;
            break;
        }
        // try to run the c file
        int runStatus = runFile(input_fd, output_fd);
        unlink("a.out");
        if (runStatus == EXECUTION_TIMED_OUT)
        {
            // grade is 20 TIMEOUT
            returnValue = 20;
            break;
        }
        // TODO to add the error message
        if (runStatus == EXECUTION_SUCCESSFUL)
        {
            // compare the output
            int compareStatus = textCompare("output", cOutputPath);
            if (compareStatus == 1)
            {
                // grade is 100 EXCELLENT
                returnValue = 100;
            }
            if (compareStatus == 2)
            {
                // grade is 75 SIMILAR
                returnValue = 75;
            }
            if (compareStatus == 3)
            {
                // grade is 50 BAD
                returnValue = 50;
            }
            // TODO to add error message
        }
    } while (0);
    close(output_fd);
    unlink("output");
    return returnValue;
}

void runOverAllFolders(char studentsFolder[MAX_STRING_SIZE], int input_fd, char cOutputPath[MAX_STRING_SIZE])
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;

    // Open the directory
    dir = opendir(studentsFolder);
    if (dir == NULL)
    {
        printf("Error opening directory\n");
        exit(EXIT_FAILURE);
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Ignore the "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Construct the full path to the file/directory
        char path[MAX_STRING_SIZE];
        strcpy(path, studentsFolder);
        strcat(path, "/");
        strcat(path, entry->d_name);

        // // Get the file info for the entry
        if (stat(path, &file_info) < 0)
        {
            printf("Error getting file info for %s\n", entry->d_name);
            continue;
        }

        // Check if the entry is a directory
        if (S_ISDIR(file_info.st_mode))
        {
            int score = grade(path, input_fd, cOutputPath);
            printf("%s ->  %d\n", entry->d_name, score);
        }
    }

    // Close the directory
    closedir(dir);
}

int main(int argc, char *argv[])
{
    int compareStatus = textCompare("files/textComparison/similar/moo/joey1.txt", "files/textComparison/similar/moo/joey2.txt");
    printf("compile status is %d\n", compareStatus);
    return 0;
}
