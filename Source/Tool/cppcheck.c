// cppcheck wrapper to display all commandline parameters
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for(int i = 0; i < argc; i++)
    {
        printf("%s%s\n", (i == 0 ? "" : " -- "), argv[i]);
    }

    return 0;
}