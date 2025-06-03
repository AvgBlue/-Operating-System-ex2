
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_STRING_SIZE 150
#define CALCULATE_ELAPSED_TIME(start_time, end_time) \
    ((end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0)

#define EXECUTION_SUCCESSFUL 0
#define EXECUTION_TIMED_OUT 1
#define EXECUTION_ERROR -1
#define EXECUTION_ERROR_COMP 255
#define RUN_TIME_FOR_TIMEOUT 5

#define COMPILE_FILE_NAME "compile_file3188.out"
#define COMPILE_FILE_NAME_LEN 20
#define GRADE_OUTPUT_FILE_NAMA "output.txt"

void addResultToFile(int result_fd, int grade, const char *name)
{
    const char *category;
    switch (grade)
    {
    case 0:
        category = ",0,NO_C_FILE\n";
        break;
    case 10:
        category = ",10,COMPILATION_ERROR\n";
        break;
    case 20:
        category = ",20,TIMEOUT\n";
        break;
    case 50:
        category = ",50,WRONG\n";
        break;
    case 75:
        category = ",75,SIMILAR\n";
        break;
    case 100:
        category = ",100,EXCELLENT\n";
        break;
    default:
        return; // Invalid number, do nothing
    }
    char result[MAX_STRING_SIZE];
    result[0] = '\0'; // Initialize result as an empty string

    // Concatenate name, grade, and category into result
    strcat(result, name);
    strcat(result, category);

    int length = strlen(result); // Get the length of the resulting string

    if (write(result_fd, result, length) == -1)
    {
        perror("Error in: write");
    }
}

int executeCommand(char *args[], int input_fd, int output_fd, int errors_fd, double *runtime,int timeout_flag)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Error in: fork");
        return EXECUTION_ERROR;
    }
    else if (pid == 0)
    {
        // Child process
        // Redirect input, output, and error to files
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("Error in: dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("Error in: dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(errors_fd, STDERR_FILENO) == -1)
        {
            perror("Error in: dup2");
            exit(EXIT_FAILURE);
        }
        // Set an alarm to terminate the child process after 10 seconds
        signal(SIGALRM, exit);
        if(timeout_flag!=0){
            alarm(RUN_TIME_FOR_TIMEOUT*2);
        }
        //  Execute the command
        execvp(args[0], args);
        // If execvp returns, it means an error occurred
        perror("Error in: execvp");
        exit(EXIT_FAILURE);
    }
    // Parent process
    int status;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    if (wait(&status) == -1)
    {
        perror("Error in: wait");
        return EXECUTION_ERROR;
    }
    // Cancel the alarm if the child process has terminated before the alarm went off
    if(timeout_flag!=0){
        alarm(0);
    }
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    *runtime = CALCULATE_ELAPSED_TIME(start_time, end_time);
    return status >> 8;
}

int textCompare(char path1[MAX_STRING_SIZE], char path2[MAX_STRING_SIZE], int errors_fd)
{
    double time;
    char *args[] = {"./comp.out", path1, path2, NULL};
    int status = executeCommand(args, STDIN_FILENO, STDOUT_FILENO, errors_fd, &time,0);
    return status;
}

int compileFile(char path[MAX_STRING_SIZE], int errors_fd)
{
    char *args[] = {"gcc", path, "-o", COMPILE_FILE_NAME, NULL};
    double time;
    int status = executeCommand(args, STDIN_FILENO, STDOUT_FILENO, errors_fd, &time,0);
    // 0 is good 1 is bad
    return status;
}

int runFile(int input_fd, int output_fd, int errors_fd)
{
    char *fileName = COMPILE_FILE_NAME;
    char runingFileString[COMPILE_FILE_NAME_LEN];
    strcpy(runingFileString, "./");
    strcat(runingFileString, fileName);

    char *args[] = {runingFileString, NULL};
    double time;
    int status = executeCommand(args, input_fd, output_fd, errors_fd, &time,1);
    if (time > RUN_TIME_FOR_TIMEOUT)
    {
        return EXECUTION_TIMED_OUT;
    }
    if (status == -1)
    {
        return EXECUTION_ERROR;
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
        perror("Error in: opendir");
        return;
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
        if (entry->d_type == DT_DIR)
        {
            continue;
        }
        // Get a pointer to the extension in the file name
        extension = entry->d_name + len - 2;

        // Check if the extension is ".c"
        if (strcmp(extension, ".c") == 0)
        {
            strcpy(c_file_path, dir_path);
            strcat(c_file_path, "/");
            strcat(c_file_path, entry->d_name);
            break;
        }
    }
    // Check for errors in readdir
    if (errno != 0)
    {
        perror("Error in readdir");
    }
    // Close the directory
    if (closedir(dir) == -1)
    {
        perror("Error in: closedir");
    }
}

int grade(char path[MAX_STRING_SIZE], char inputFilePath[MAX_STRING_SIZE], char cOutputPath[MAX_STRING_SIZE], int errors_fd)
{
    int returnValue = 0;
    char outputPath[MAX_STRING_SIZE] = GRADE_OUTPUT_FILE_NAMA;
    int output_fd = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == EXECUTION_ERROR)
    {
        perror("Error in: open");
        return EXECUTION_ERROR;
    }
    int input_fd = open(inputFilePath, O_RDONLY);
    if (input_fd == EXECUTION_ERROR)
    {
        perror("Error in: open");
        return EXECUTION_ERROR;
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
        int compileStatus = compileFile(cfile, errors_fd);
        if (compileStatus == 1)
        {
            // grade is 10 COMPILATION_ERROR
            returnValue = 10;
            break;
        }
        // try to run the c file
        int runStatus = runFile(input_fd, output_fd, errors_fd);
        if (runStatus == EXECUTION_ERROR)
        {
            return EXECUTION_ERROR;
            break;
        }
        if (close(output_fd) == -1)
        {
            perror("Error in: close");
        }
        if (unlink(COMPILE_FILE_NAME) == -1)
        {
            perror("Error in: unlink");
        }
        if (runStatus == EXECUTION_TIMED_OUT)
        {
            // grade is 20 TIMEOUT
            returnValue = 20;
            break;
        }
        if (runStatus == EXECUTION_SUCCESSFUL)
        {
            // compare the output
            int compareStatus = textCompare(outputPath, cOutputPath, errors_fd);
            if (compareStatus == EXECUTION_ERROR_COMP)
            {
                // we will skip filling the result file
                perror("Error in: comp.out");
                return EXECUTION_ERROR;
            }
            if (compareStatus == 1)
            {
                // grade is 100 EXCELLENT
                returnValue = 100;
            }
            if (compareStatus == 3)
            {
                // grade is 75 SIMILAR
                returnValue = 75;
            }
            if (compareStatus == 2)
            {
                // grade is 50 BAD
                returnValue = 50;
            }
        }
    } while (0);
    if (unlink(outputPath) == -1)
    {
        perror("Error in: unlink");
    }
    if (close(input_fd) == -1)
    {
        perror("Error in: close");
    }
    return returnValue;
}

void runOverAllFolders(char studentsFolder[MAX_STRING_SIZE], char inputFilePath[MAX_STRING_SIZE], char cOutputPath[MAX_STRING_SIZE], int result_fd, int errors_fd)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;

    // Open the directory
    dir = opendir(studentsFolder);
    if (dir == NULL)
    {
        perror("Error in: opendir");
        return;
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
            perror("Error in: stat");
            continue;
        }

        // Check if the entry is a directory
        if (S_ISDIR(file_info.st_mode))
        {
            int score = grade(path, inputFilePath, cOutputPath, errors_fd);
            if (score != EXECUTION_ERROR)
            {
                // everything is ok in the grading
                addResultToFile(result_fd, score, entry->d_name);
            }
        }
    }

    // Close the directory
    if (closedir(dir) == -1)
    {
        perror("Error in: closedir");
    }
}

int lineCountInFile(char buffer[MAX_STRING_SIZE * 4])
{
    int count = 0;
    for (int i = 0; i < MAX_STRING_SIZE * 4; i++)
    {
        count += buffer[i] == '\n';
    }
    return count;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("Not a valid directory");
        return EXECUTION_ERROR;
    }
    char confFile[MAX_STRING_SIZE];

    strcpy(confFile, argv[1]);

    int fdConf = open(confFile, O_RDONLY);
    if (fdConf == -1)
    {
        perror("Not a valid directory");
        return EXECUTION_ERROR;
    }
    char buffer[MAX_STRING_SIZE * 4];
    ssize_t nread;
    nread = read(fdConf, buffer, sizeof(buffer));
    if (nread == -1)
    {
        perror("Error in: read");
        if (close(fdConf) == -1)
        {
            perror("Error in: close");
        }
        return EXECUTION_ERROR;
    }
    if (close(fdConf) == -1)
    {
        perror("Error in: close");
        return EXECUTION_ERROR;
    }
    char studentsFolder[MAX_STRING_SIZE];
    char correct_outputFile[MAX_STRING_SIZE];
    char inputFile[MAX_STRING_SIZE];
    // first line
    char *token = strtok(buffer, "\n");
    strcpy(studentsFolder, token);
    // second line
    token = strtok(NULL, "\n");
    strcpy(inputFile, token);
    // third line
    token = strtok(NULL, "\n");
    strcpy(correct_outputFile, token);
    if (access(correct_outputFile, F_OK) == -1 || access(inputFile, F_OK) == -1 || access(studentsFolder, F_OK) == -1)
    {
        // one of the path didn't exist
        perror("Error in: access");
        goto resultAndErrorDidntOpenYet;
    }

    int result_fd = open("results.csv", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (result_fd == -1)
    {
        perror("Error in: open");
        goto resultIsOpen;
    }
    int errors_fd = open("errors.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (errors_fd == -1)
    {
        perror("Error in: open");
        goto errorIsOpen;
    }
    // run over all of the folders in the students folder
    runOverAllFolders(studentsFolder, inputFile, correct_outputFile, result_fd, errors_fd);

errorIsOpen:
    if (close(errors_fd) == -1)
    {
        perror("Error in: close");
    }
resultIsOpen:
    if (close(result_fd) == -1)
    {
        perror("Error in: close");
    }
resultAndErrorDidntOpenYet:
    return 0;
}
