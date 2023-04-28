#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define EXIT_EXECVP_FAILED 99
#define MAX_SIZE 100
#define MAX_STRING_SIZE 100

typedef struct
{
	char queue[MAX_SIZE][MAX_STRING_SIZE + MAX_STRING_SIZE];
	int front;
	int rear;
	int count;
} Queue;

// initializes the queue
void initQueue(Queue *q)
{
	// set front to 0
	q->front = 0;
	// set rear to -1
	q->rear = -1;
	// set count 0;
	q->count = 0;
}

// This function adds an element to the queue
void push(Queue *q, char *value)
{
	// If the queue is full, return
	if (q->count == MAX_SIZE)
	{
		return;
	}

	// Increment the rear pointer and add the value to the queue
	q->rear = (q->rear + 1) % MAX_SIZE;
	strcpy(q->queue[q->rear], value);
	q->count++;
}

// pop element from the queue
void pop(Queue *q)
{
	// Check if the queue is empty
	if (q->count == 0)
	{
		return; // If empty, return without making any changes
	}

	// Update the front index of the queue to remove the element
	q->front = (q->front + 1) % MAX_SIZE; // Circular buffer implementation

	// Decrease the count of elements in the queue
	q->count--;
}

// return the number of elements in the queue
int queueCount(Queue *q)
{
	// Return the count of elements in the queue
	return q->count;
}

// printing the history Queue
void printHistory(Queue *q)
{
	// Check if the queue is empty
	if (q->count == 0)
	{
		// If empty, return without printing anything
		return;
	}
	// Initialize index to the front of the queue
	int i = q->front;
	// Initialize counter for elements printed
	int elementsPrinted = 0;

	// Loop through the queue and print the elements
	while (elementsPrinted < q->count)
	{
		// Print the element at the current index
		printf("%s\n", q->queue[i]);

		// Move to the next index, considering circular buffer
		i = (i + 1) % MAX_SIZE;

		elementsPrinted++; // Increase the counter for elements printed
	}
}

// add elements to the history queue
void addToHistory(Queue *q, pid_t pid, char *str)
{
	if (queueCount(q) == MAX_SIZE)
	{
		// If the queue is full, remove the oldest element from the front
		pop(q);
	}
	// Create a new string buffer for the value
	char value[MAX_STRING_SIZE] = " ";
	// Format the process ID and string into the value buffer
	sprintf(value, "%d %s", pid, str);
	// Add the new value to the end of the queue
	push(q, value);
}

// convert the string to an array and return the number words the array
void stringToArray(const char str[], char arr[][MAX_STRING_SIZE], int *numWords)
{
	// Create a temporary copy of the input string to avoid modifying the original string
	char copyStr[100];
	strcpy(copyStr, str);

	// Use strtok to tokenize the string by space (" ")
	char *token = strtok(copyStr, " ");

	// Loop until all tokens (words) are processed or numWords reaches the maximum limit
	while (token != NULL && *numWords < 100)
	{
		// Copy the token (word) into the array of strings at the current index
		strcpy(arr[*numWords], token);
		// Increment the counter for number of words
		(*numWords)++;
		// Move to the next token (word)
		token = strtok(NULL, " ");
	}
}

// input the a command from the user
void inputCommand(char str[MAX_STRING_SIZE])
{
	// Print prompt for input
	printf("$ ");
	// Flush output buffer to ensure prompt is displayed
	fflush(stdout);
	// Read input from user, allowing up to 100 characters until newline or carriage return
	scanf(" %100[^\n\r]", str);
}

// excuteing the commants the the user inputed
void executeCommand(char str[MAX_STRING_SIZE], char arr[MAX_SIZE][MAX_STRING_SIZE], int numWords, Queue *historyQueue)
{
	// Fork a child process
	pid_t pidfork = fork();
	if (pidfork == -1)
	{
		// Fork failed
		// Print error message
		perror("fork failed");
		return;
	}
	else if (pidfork == 0)
	{
		// Child process
		// Allocate memory for argument array
		char **args = (char **)calloc(numWords + 1, sizeof(char *));
		for (int i = 0; i < numWords; i++)
		{
			// Copy arguments to argument array
			args[i] = arr[i];
		}
		// Set last element of argument array to NULL
		args[numWords] = NULL;
		// Execute the command with arguments
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
	// Add executed command to history
	addToHistory(historyQueue, pidfork, str);
}

// the implementation of cd
int cd(char str[MAX_STRING_SIZE], char arr[MAX_SIZE][MAX_STRING_SIZE], int numWords, Queue *historyQueue)
{
	if (strcmp(arr[0], "cd") != 0)
	{
		// Return failure status if command is not "cd"
		return 0;
	}
	// Add executed command to history
	addToHistory(historyQueue, getpid(), str);

	if (numWords > 2)
	{
		// Print error message for too many arguments
		printf("cd: too many arguments\n");
		// return for continue
		return 1;
	}
	// Special cases for "." and ".."
	if (strcmp(arr[1], ".") == 0 || strcmp(arr[1], "..") == 0)
	{
		// Change directory to "." or ".."
		chdir(arr[1]);
		// return for continue
		return 1;
	}
	// Regular attempt to change directory
	if (chdir(arr[1]) == -1)
	{
		// Print error message for invalid directory
		printf("cd: too many arguments\n");
	}
	// return for continue
	return 1;
}

// the implementation of history
int history(char str[MAX_STRING_SIZE], char arr[MAX_SIZE][MAX_STRING_SIZE], int numWords, Queue *historyQueue)
{
	// history implementation
	if (strcmp(arr[0], "history") != 0)
	{
		return 0;
	}
	// Add executed command to history
	addToHistory(historyQueue, getpid(), str);

	if (numWords > 1)
	{
		// Print error message for too many arguments
		printf("history: too many arguments\n");
		// return for continue
		return 1;
	}
	// Print history
	printHistory(historyQueue);
	// return for continue
	return 1;
}

// setup the path for the shell
void settingPath(int argc, char *argv[])
{
	// Get the current PATH environment variable value
	char *value = getenv("PATH");
	// Calculate the size of the PATH string
	int path_size = strlen(value) + 1;
	// Allocate memory for the PATH string
	char *path = (char *)calloc(path_size, sizeof(char));
	// Copy the current PATH string to the newly allocated memory
	strcpy(path, value);

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "\0") == 0)
		{
			// Skip if argument is empty
			continue;
		}
		// Calculate the new size of the PATH string with the additional argument
		int size = strlen(path) + strlen(argv[i]) + 10;
		if (size >= path_size)
		{
			// Resize the PATH string if needed
			path = (char *)realloc(path, size * sizeof(char));
			path_size = size;
		}
		strcat(path, ":");
		strcat(path, argv[i]);
	}
	// Set the new PATH value
	setenv("PATH", path, 1);
	// Free the allocated memory for PATH string
	free(path);
}

// the main function
int main(int argc, char *argv[])
{
	// Set the PATH environment variable using command-line arguments
	settingPath(argc, argv);
	// Create a historyQueue of type Queue to store command history
	Queue historyQueue;
	// Initialize the historyQueue
	initQueue(&historyQueue);

	while (1)
	{
		// Initialize a string buffer to store user input
		char str[MAX_STRING_SIZE] = " ";
		// Initialize a 2D array to store parsed input words
		char arr[MAX_SIZE][MAX_STRING_SIZE];
		// Initialize a counter for the number of words in the input command
		int numWords = 0;
		// Get user input and store it in the str buffer
		inputCommand(str);
		// Parse the input command into individual words and store them in the arr array
		stringToArray(str, arr, &numWords);

		if (strcmp(arr[0], "exit") == 0)
		{
			// If the command is "exit", break out of the loop and exit the program
			break;
		}

		if (cd(str, arr, numWords, &historyQueue) || history(str, arr, numWords, &historyQueue))
		{
			// If the command is "cd" or "history", handle it and continue to the next iteration of the loop
			continue;
		}
		// Execute the command using the executeCommand() function
		executeCommand(str, arr, numWords, &historyQueue);
	}
	// Exit the program with a return value of 0 (indicating successful execution)
	return 0;
}
