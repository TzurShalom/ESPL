#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int digitCounter(char* str) 
{
    int count = 0;
    int i=0;
    char val =str[i];
    for (i = 0; val != '\0'; i++) 
    {
        if (val >= '0' && val <= '9') 
        {
            val =str[i+1];
            count++;
        }
    }
    return count;
}

int main(int argc, char **argv)
{
    if (argc == 1) 
    {
       fprintf(stderr,"Error:no string to count"); 
    }
    else 
    {
        int string=digitCounter(argv[1]);
        printf("Number of digits is: %d.\n",string);
    }
    return 0;
}