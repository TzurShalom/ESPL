#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int debug = 0;

void execute(cmdLine *pCmdLine)
{
   if (strcmp(pCmdLine->arguments[0],"quit") == 0) 
   {
      _exit(0);
   }
   else if (strcmp(pCmdLine->arguments[0],"cd") == 0)
   {
      if (chdir(pCmdLine->arguments[1]) == -1) 
      {
        perror("the cd operation fails");
        _exit(1);
      }
   }
      else if (strcmp(pCmdLine->arguments[0],"suspend") == 0)
   {
      if (kill(atoi(pCmdLine->arguments[1]),SIGTSTP) == -1)
      {
        perror("the kill operation fails");
        _exit(1);
      }   
   }
   else if (strcmp(pCmdLine->arguments[0],"wake") == 0)
   {
      if (kill(atoi(pCmdLine->arguments[1]),SIGCONT) == -1)
      {
        perror("the kill operation fails");
        _exit(1);
      }          
   }
   else if (strcmp(pCmdLine->arguments[0],"kill") == 0)
   {
      if (kill(atoi(pCmdLine->arguments[1]),SIGINT) == -1)
      {
         perror("the kill operation fails");
         _exit(1);
      }    
   }
   else
   {
        int * sta = NULL;
        int pid = fork();

        if (pid > 0)
        {
           if (pCmdLine->blocking == 1)
           {
               if (waitpid(pid,sta,0) == -1)
               {
                   perror("the wait operation fails");
                   _exit(1);
               }
           }
        }    
        else if (pid == 0)
        {
            int input = -1;
            int output = -1;

            if (debug)
            {
                fprintf(stderr,"PID : %i\n",getpid());
                fprintf(stderr,"Executing command : %s\n",pCmdLine->arguments[0]);
            }

            if (pCmdLine->inputRedirect != NULL)
            {
                input = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777 );

                if (input == -1)
                {
                    perror("the open operation fails");
                    _exit(1);
                }
                if (dup2(input,STDIN_FILENO) == -1)
                {
                    perror("the dup2 operation fails");
                    _exit(1);
                }

            }

            if (pCmdLine->outputRedirect != NULL)
            {
                output = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777 );

                if (output == -1)
                {
                    perror("the open operation fails");
                    _exit(1);
                }
                if (dup2(output,STDOUT_FILENO) == -1)
                {
                    perror("the dup2 operation fails");
                    _exit(1);
                }            
            }

            if (execvp(pCmdLine->arguments[0],pCmdLine->arguments) == -1) // Terminates the current process
            {
                perror("the execv operation fails");
                _exit(1);
            }
        }
   }
}

int main(int argc, char ** argv) 
{ 
   if ((argc > 1) && (strcmp(argv[1],"-d") == 0)) {debug = 1;}

   while (1)
   {
      char cwd[PATH_MAX];
      getcwd(cwd, sizeof(cwd));
      printf("%s ", cwd);

      char line[2048];
      fgets(line,sizeof(line),stdin);

      cmdLine *cmdline = parseCmdLines(line);

      execute(cmdline);
      freeCmdLines(cmdline);
   }  
   return 0;
}
