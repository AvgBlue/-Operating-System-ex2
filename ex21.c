

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct FileReader
{
    // file descriptor
    int fd;
    // buffer
    char buf[BUFSIZ];
    // number of byte read in the buffer
    ssize_t nread;
} FileReader;

void print_error(const char *message)
{
    write(STDERR_FILENO, message, strlen(message));
}

FileReader openFile(const char *path)
{
    FileReader fr;
    // Open the file for reading
    fr.fd = open(path, O_RDONLY);
    if (fr.fd == -1)
    {
        print_error("open");
        exit(EXIT_FAILURE);
    }
    // Get the size of the file
    fr.nread = 0;
    return fr;
}

void closeFile(FileReader *fr)
{
    close(fr->fd);
}

// read a part of the file into the buffer
void readPart(FileReader *fr)
{
    fr->nread = read(fr->fd, fr->buf, BUFSIZ);
    if (fr->nread < 0)
    {
        print_error("read");
    }
}

bool isSameChar(char c1, char c2)
{
    if (c1 == c2)
    {
        return true;
    }
    if (c1 >= 'a' && c1 <= 'z' && c2 == c1 - ('a' - 'A'))
    {
        return true;
    }
    if (c1 >= 'A' && c1 <= 'Z' && c2 == c1 + ('a' - 'A'))
    {
        return true;
    }
    return false;
}

int compareFiles(FileReader *fr1, FileReader *fr2)
{
    bool identical = true;
    bool similar = true;
    readPart(fr1);
    readPart(fr2);
    int i = 0;
    int j = 0;
    while (fr1->nread > 0 && fr2->nread > 0)
    {
        if (i >= fr1->nread)
        {
            readPart(fr1);
            if (!fr1->nread > 0)
                break;
            i = 0;
        }
        if (j >= fr2->nread)
        {
            readPart(fr2);
            if (!fr2->nread > 0)
                break;
            j = 0;
        }
        // still identical
        if (fr1->buf[i] == fr2->buf[j])
        {
            i++;
            j++;
            continue;
        }
        // don't identical
        identical = false;

        if (fr1->buf[i] == ' ' || fr1->buf[i] == '\n' || fr1->buf[i] == '\t' || fr1->buf[i] == '\r')
        {
            i++;
            continue;
        }
        if (fr2->buf[j] == ' ' || fr2->buf[j] == '\n' || fr2->buf[j] == '\t' || fr2->buf[j] == '\r')
        {
            j++;
            continue;
        }
        if (!isSameChar(fr1->buf[i], fr2->buf[j]))
        {
            similar = false;
            break;
        }
        i++;
        j++;
    }
    while (fr1->nread > 0)
    {
        if (i >= fr1->nread)
        {
            readPart(fr1);
            if (!fr1->nread > 0)
                break;
            i = 0;
        }
        identical = false;
        if (fr1->buf[i] == ' ' || fr1->buf[i] == '\n' || fr1->buf[i] == '\t' || fr1->buf[i] == '\r')
        {
            i++;
            continue;
        }
        similar = false;
        i++;
    }
    while (fr2->nread > 0)
    {
        if (j >= fr2->nread)
        {
            readPart(fr2);
            if (!fr2->nread > 0)
                break;
            j = 0;
        }
        identical = false;
        if (fr2->buf[j] == ' ' || fr2->buf[j] == '\n' || fr2->buf[j] == '\t' || fr2->buf[j] == '\r')
        {
            j++;
            continue;
        }
        similar = false;
        j++;
    }
    if (identical)
    {
        return 1;
    }
    if (similar)
    {
        return 2;
    }
    return 3;
}

int main(int argc, char *argv[])
{
    // Check that an argument was provided
    if (argc != 3)
    {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FileReader fr1 = openFile(argv[1]);
    FileReader fr2 = openFile(argv[2]);
    int value = compareFiles(&fr1, &fr2);
    closeFile(&fr1);
    closeFile(&fr2);
    return value;
}
