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

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

process *process_list = NULL;

//--------------------------------------------//

#define HISTLEN 20
char *history [HISTLEN];
int newest = 0; // newest indicates the "index of the newest" item in queue (newest-1 in the array)
int oldest = 0; // oldest indicates the "index of the oldest" item in queue (oldest-1 in the array)

//--------------------------------------------//

void addToHistory(char *cl)
{
    char *replace = malloc(strlen(cl) );
    strcpy(replace, cl);
    if (oldest == newest && history[newest] != NULL)
    {
        free(history[oldest]);
        oldest = (oldest + 1) % HISTLEN;
    }

    history[newest] = replace;
    newest = (newest + 1) % HISTLEN;
}

//--------------------------------------------//

int sizeOfHistory()
{
    return ( newest- oldest + HISTLEN) % HISTLEN + 1;
}

//--------------------------------------------//

char* getHistoryByIndex(int n)
{
    int index = (oldest + n - 1) % HISTLEN;
    if (n < 0 || n > HISTLEN || history[index] == NULL)
    {
        fprintf(stdout, "Invalid history index\n");
        _exit(1);
    }
    return history[index];
}

//--------------------------------------------//

void deleteFromHistory()
{
    free(history[oldest]);
    oldest = (oldest - 1) % HISTLEN;
}

//--------------------------------------------//

void freeHistory()
{
    for (int i = 0; i < HISTLEN; i++)
    {
        if (history[i]!=NULL)
        {
            free(history[i]);
        }
    } 
}

//--------------------------------------------//

void freeProcessList(process *process_list)
{
    process *p = process_list;
    while (process_list != NULL)
    {
        process_list = process_list->next;
        freeCmdLines(p->cmd);
        free(p);
        p = process_list;
    }
}

//--------------------------------------------//

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *p = process_list;

    while (p != NULL && p->pid != pid)
    {
        p = p->next;
    }

    if (p->pid == pid)
    {
        p->status = status;
    }
}

//--------------------------------------------//

void updateProcessList(process **process_list)
{
  process* p = *process_list;
    int sta;

    while (p != NULL)
    {      
        pid_t child_pid = waitpid(p->pid, &sta,WNOHANG | WUNTRACED | WCONTINUED);
        if(child_pid == TERMINATED)
        {
            p->status=TERMINATED;
        }
        if(child_pid >0)
        {
            if (WIFEXITED(sta) || WIFSIGNALED(sta)) 
            {
                p->status=TERMINATED;
            }
            else if(WIFCONTINUED(sta))
            {
                p->status=RUNNING;
            }
            else if(WIFSTOPPED(sta))
            {
                p->status=SUSPENDED;
            }
        }  
        p = p->next;
    }
}

//--------------------------------------------//

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *p = malloc(sizeof(process));
    p->cmd = cmd;
    p->next = *process_list;
    p->pid = pid;
    p->status = RUNNING;
    *process_list = p;
}

//--------------------------------------------//

void printProcessList(process **process_list)
{
    updateProcessList(process_list);
    process *current = *process_list;
    process *previous = NULL;
    process *remove = NULL;
    int index = 1;
    printf("Index     PID     Status     Command\n");
    while (current != NULL)
    {
        char *statusN;

        if (current->status == 0)
        {
            statusN = "SUSPENDED";
        }
        else if (current->status == -1)
        {
            statusN = "TERMINATED";
        }
        else if (current->status == 1)
        {
            statusN = "RUNNING";
        }
        
        printf("%d     %d     %s     %s ", index, current->pid, statusN, current->cmd->arguments[0]); // status

        for (int i = 1; i < current->cmd->argCount; i++)
        {
            printf(" %s", current->cmd->arguments[i]);
        }

        printf("\n");

        if (current->status == TERMINATED)
        {
            if (previous != NULL)
            {
                previous->next = current->next;
                remove = current;
                current = current->next;
            }
            else
            {
                remove = current;
                current = current->next;
                *process_list = current;
            }
            
            freeCmdLines(remove->cmd);
            free(remove);
        }
        else
        {
            previous = current;
            current = current->next;
        }
        index++;
    }
}

//--------------------------------------------//

int pipeline(cmdLine *pCmdLine)
{
    int *sta = NULL;
    int fd[2];
    
    if (pipe(fd) == -1) // creates a pipe and sets fd[0] as the read end and fd[1] as the write end
    {
        perror("the pipe operation fails\n");
        _exit(1);
    }
    
    fprintf(stderr, "parent_process > forking child1\n");
    int child1 = fork();
    
    if (child1 == 0)
    {
        fprintf(stderr, "child1>redirecting stdout to the write end of the pipe…\n");
        close(1);
        dup(fd[1]);

        if (pCmdLine->inputRedirect != NULL)
        {
            close(0);
            
            if (open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777) == -1)
            {
                perror("the open operation fails");
                freeCmdLines(pCmdLine);
                _exit(1);
            }
        }
        if (pCmdLine->outputRedirect != NULL)
        {
            close(1);
            
            if (open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777) == -1)
            {
                perror("the open operation fails");
                freeCmdLines(pCmdLine);
                _exit(1);
            }
        }
        
        close(fd[1]);
        fprintf(stderr, "child1 > going to execute cmd:");
        
        for (int i = 1; i < pCmdLine->argCount; i++)
        {
            printf(" %s", pCmdLine->arguments[i]);
        }
        
        execvp( pCmdLine->arguments[0], pCmdLine->arguments);
        _exit(1); // error
    }
    else if (child1 > 0)
    {
        fprintf(stderr, "%s%i\n", "parent_process > created process with id: ", child1);
        fprintf(stderr, "%s\n", "parent_process>closing the write end of the pipe…");
        close(fd[1]);
        pCmdLine = pCmdLine->next;
        fprintf(stderr, "parent_process > forking child2\n");
        int child2 = fork();
        
        if (child2 == 0)
        {
            close(0);
            fprintf(stderr, "\nchild2 > redirecting stdin to the read end of the pipe\n");
            dup(fd[0]);
            close(fd[0]);

            if (pCmdLine->inputRedirect != NULL)
            {
                close(0);
                
                if (open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777) == -1)
                {
                    perror("the open operation fails");
                    freeCmdLines(pCmdLine);
                    _exit(1);
                }
            }
            if (pCmdLine->outputRedirect != NULL)
            {
                close(1);
                
                if (open(pCmdLine->outputRedirect, O_RDWR, 0777) == -1)
                {
                    perror("the open operation fails");
                    freeCmdLines(pCmdLine);
                    _exit(1);
                }
            }
            
            fprintf(stderr, "child1 > going to execute cmd:");
            
            for (int i = 1; i < pCmdLine->argCount; i++)
            {
                printf(" %s", pCmdLine->arguments[i]);
            }
            
            execvp( pCmdLine->arguments[0],  pCmdLine->arguments);
            _exit(1);
        }
        else if (child2 > 0)
        {
            fprintf(stderr, "%s%i\n", "parent_process > created process with id: ", child2);
            close(fd[0]);
            fprintf(stderr, "parent_process>closing the read end of the pipe…\n");
            fprintf(stderr, "parent_process > waiting for child processes to terminate\n");
            waitpid(child1, sta, 0);
            fprintf(stderr, "parent_process > waiting for child processes to terminate\n");
            waitpid(child2, sta, 0);
            fprintf(stderr, "parent_process > exiting\n");
        }
    }
    return 0;
}

//--------------------------------------------//

void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
    {
        freeProcessList(process_list);
        freeCmdLines(pCmdLine);
        freeHistory();
        _exit(0);
    }
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            perror("the cd operation fails");
            freeProcessList(process_list);
            _exit(1);
        }  
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
    {
        printProcessList(&process_list);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        if (kill(atoi(pCmdLine->arguments[1]), SIGTSTP) == -1)
        {
            perror("the kill(suspend) operation fails");
            freeProcessList(process_list);
            _exit(1);
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
    {
        if (kill(atoi(pCmdLine->arguments[1]), SIGCONT) == -1)
        {
            perror("the kill(wake) operation fails");
            freeProcessList(process_list);
            _exit(1);
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "kill") == 0)
    {
        if (kill(atoi(pCmdLine->arguments[1]), SIGINT) == -1)
        {
            perror("the kill(kill) operation fails");
            freeProcessList(process_list);
            _exit(1);
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "history") == 0)
    {
        if ( newest==oldest){
            int index=0;
            for (int i = oldest; index < HISTLEN  ; i++)
            {
                printf("command %d: %s\n",index+1, history[i%HISTLEN]);
                index++;
            }
        }
        else
        { 
            for (int i = oldest; i < newest ; i++)
            {
                printf("command %d: %s\n",i+1, history[i]);
            }
        }
        freeCmdLines(pCmdLine);
    }
    else
    {
        int *sta = NULL;
        int pid = fork();
        if (pid > 0)
        {
            addProcess(&process_list, pCmdLine, pid); // pid
            if (pCmdLine->blocking == 1)
            {
                if (waitpid(pid, sta, 0) == -1)
                {
                    perror("the wait operation fails");
                    freeProcessList(process_list);
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
                fprintf(stderr, "\nPID : %i\n", getpid());
                fprintf(stderr, "Executing command : %s\n", pCmdLine->arguments[0]);
            }

            if (pCmdLine->inputRedirect != NULL)
            {
                input = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777);

                if (input == -1)
                {
                    perror("the open operation fails");
                    freeProcessList(process_list);
                    _exit(1);
                }
                if (dup2(input, STDIN_FILENO) == -1)
                {
                    perror("the dup2 operation fails");
                    freeProcessList(process_list);
                    _exit(1);
                }
            }

            if (pCmdLine->outputRedirect != NULL)
            {
                output = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);

                if (output == -1)
                {
                    perror("the open operation fails");
                    freeProcessList(process_list);
                    _exit(1);
                }
                if (dup2(output, STDOUT_FILENO) == -1)
                {
                    perror("the dup2 operation fails");
                    freeProcessList(process_list);
                    _exit(1);
                }
            }

            if (pCmdLine->next != NULL)
            {
                pipeline(pCmdLine);
            }
            else
            {
                if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
                {
                    perror("the execvp operation fails");
                    freeProcessList(process_list);
                    _exit(1);
                }
            }
        }
    }
}

//--------------------------------------------//

int main(int argc, char **argv)
{
    if ((argc > 1) && (strcmp(argv[1], "-d") == 0))
    {
        debug = 1;
    }

    while (1)
    {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        printf("%s ", cwd);

        char line[2048];
        fgets(line, sizeof(line), stdin);
        cmdLine *cmdline = parseCmdLines(line);
        if (strcmp(cmdline->arguments[0], "!!") == 0)
        {
            int index = ((HISTLEN+ newest-1) % HISTLEN);
            printf("%d",index);
            char *unparse = getHistoryByIndex(index);
            cmdLine *parsedLine = parseCmdLines(unparse);
            execute(parsedLine);
            addToHistory(unparse);
        }
        else if (cmdline->arguments[0][0] == '!')
        {
            int index = atoi(&cmdline->arguments[0][1]);
            char *unparse = getHistoryByIndex(index);
            cmdLine *parsedLine = parseCmdLines(unparse);
            execute(parsedLine);
            addToHistory(unparse);
        }
                
        else if (strcmp(line, "\n") != 0)
        {   
            addToHistory(line);
            execute(cmdline);
        }
    }
    freeHistory(history);
    free(history);
    freeProcessList(process_list);
    free(process_list);
    return 0;
}
