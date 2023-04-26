

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

FileReader openFile(const char *path)
{
    FileReader fr;
    // Open the file for reading
    fr.fd = open(path, O_RDONLY);
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
        perror("read");
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

bool compareFiles(FileReader *fr1, FileReader *fr2)
{
    bool same = true;
    do
    {
        readPart(fr1);
        readPart(fr2);
        long unsigned int bytesread = MIN(fr1->nread, fr2->nread);
        if (memcmp(fr1->buf, fr2->buf, bytesread) != 0)
        {
            same = false;
            break;
        }
    } while (fr1->nread > 0 && fr2->nread > 0);
    if (fr1->nread != fr2->nread)
    {
        same = false;
    }
    return same;
}

bool compareFilesForIdentical(FileReader *fr1, FileReader *fr2)
{
    bool same = true;
    do
    {
        readPart(fr1);
        readPart(fr2);
        long unsigned int bytesread = MIN(fr1->nread, fr2->nread);
        if (memcmp(fr1->buf, fr2->buf, bytesread) != 0)
        {
            same = false;
            break;
        }
    } while (fr1->nread > 0 && fr2->nread > 0);
    if (fr1->nread != fr2->nread)
    {
        same = false;
    }
    return same;
}

bool compareFilesForSimilar(FileReader *fr1, FileReader *fr2)
{

    bool similar = true;
    readPart(fr1);
    readPart(fr2);
    fwrite(fr1->buf, 1, fr1->nread, stdout);
    fwrite(fr2->buf, 1, fr2->nread, stdout);
    int i = 0;
    int j = 0;
    while (fr1->nread > 0 || fr2->nread > 0)
    {
        if (i >= fr1->nread)
        {
            readPart(fr1);
            i = 0;
        }
        if (j >= fr2->nread)
        {
            readPart(fr2);
            j = 0;
        }
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
    if (fr1->nread != fr2->nread)
    {
        similar = false;
    }
    return similar;
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

    if (compareFilesForIdentical(&fr1, &fr2))
    {
        printf("Files are the Identical\n");
        closeFile(&fr1);
        closeFile(&fr2);
        return 0;
    }
    closeFile(&fr1);
    closeFile(&fr2);
    fr1 = openFile(argv[1]);
    fr2 = openFile(argv[2]);
     if (compareFilesForSimilar(&fr1, &fr2))
    {
        printf("Files are similar\n");
    }
    else
    {
        printf("Files are different\n");
    }
    closeFile(&fr1);
    closeFile(&fr2);
    return 0;
}
