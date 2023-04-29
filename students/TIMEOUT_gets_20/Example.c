#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
int main()
{
    for (int i = 1; i <= 8; i++)
    {
        sleep(1);
        printf("%d\n",i);
        fflush(stdout);
    }
}
