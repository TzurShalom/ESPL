#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <sys/wait.h>

int main(int argc, char ** argv)
{
    int * sta = NULL;

    int fd[2];
    if (pipe(fd) == -1) // creates a pipe and sets fd[0] as the read end and fd[1] as the write end
    {
        perror("the pipe operation fails\n");
        _exit(1);
    } 

    fprintf(stderr,"parent_process > forking child1\n");
    int child1 = fork(); 

    if (child1 == 0) 
    {   
        fprintf(stderr,"child1>redirecting stdout to the write end of the pipe…\n");

        close(1);
        dup(fd[1]);
        close(fd[1]); 

        char * cmd[] = {"ls","-l",NULL};
        fprintf(stderr,"child1 > going to execute cmd: ls -l\n");
        execvp(cmd[0],cmd);
        _exit(1);//error
    }
    else if (child1 > 0)
    {   
        fprintf(stderr,"%s%i\n","parent_process > created process with id: ", child1);
        fprintf(stderr,"%s\n","parent_process>closing the write end of the pipe…");
        
        close(fd[1]);
        fprintf(stderr,"parent_process > forking child2\n");
       
        int child2 = fork();
       
        if (child2 == 0)
        {
            close(0);
            fprintf(stderr,"\nchild2 > redirecting stdin to the read end of the pipe\n");
            dup(fd[0]);
            close(fd[0]);

            char * cmd[] = {"tail","-n","2", NULL};
            fprintf(stderr,"child2 > going to execute cmd: tail -n 2\n");
            execvp(cmd[0],cmd);
            _exit(1);
        }
        else if (child2 > 0)
        { 
            fprintf(stderr,"%s%i\n","parent_process > created process with id: ", child2);
            close(fd[0]);
            fprintf(stderr, "parent_process>closing the read end of the pipe…");
            fprintf(stderr,"parent_process > waiting for child processes to terminate\n");
            waitpid(child1,sta,0);
            fprintf(stderr,"parent_process > waiting for child processes to terminate\n");
            waitpid(child2,sta,0);
            
            fprintf(stderr,"parent_process > exiting\n");
        }
    }  
    return 0;
}
