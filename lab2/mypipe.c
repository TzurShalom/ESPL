#include <unistd.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    int fd[2];
    if (pipe(fd) == -1) // creates a pipe and sets fd[0] as the read end and fd[1] as the write end
    {
        perror("the pipe operation fails");
        _exit(1);
    } 

    int pid = fork();

    if (pid == 0) 
    {
        close(fd[0]);
        char* output = "hello";
        write(fd[1], output, 16);
        close(fd[1]);
    }

    if (pid > 0)
    {
        close(fd[1]);
        char input[16];
        read(fd[0], input, 16);
        printf("%s\n", input);
        close(fd[0]);
    }
}
