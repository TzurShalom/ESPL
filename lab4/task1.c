#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

//----------------------------------------------------------------------------------//

typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_flag;  
} state;

//----------------------------------------------------------------------------------//

struct fun_desc {
char *name;
void (*fun)(state*);
};

//----------------------------------------------------------------------------------//

void resetS(state *s){
    s->debug_mode ='0';
    s->unit_size = 1;
    s->display_flag = '0';
}

//----------------------------------------------------------------------------------//

void ToggleDebugMode(state* s)
{
    char debugFlag=s->debug_mode;
    if (debugFlag=='0') 
    {
        s->debug_mode='1';
        printf("Debug: Debug flag now on");
    }
    else if (debugFlag=='1')
    {
        s->debug_mode='0';
        printf("Debug: Debug flag now off");
    }
}

void SetFileName(state* s)
{
    char filename[100];
    printf("Please enter file name: ");
    fgets(filename, 100 ,stdin);
    sscanf(filename , "%s", s->file_name);
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "Debug: file name set to %s ",filename );
    }
}

void SetUnitSize(state* s)
{
    char c[8];
    printf("Please enter unit size: ");
    fgets(c,8,stdin);
    int number = atoi(c);
    if (number == 4 || number == 1 || number == 2)
    {
        if (s->debug_mode=='1')
        {
            fprintf(stderr, "Debug: set size to %d ",number);
        }
        s->unit_size=number;
    }
    else
    {
        fprintf(stderr,"SetUnitSize: Error not valid input.\n");
    }
}

void LoadIntoMemory(state* s)
{
    char input[100];
    char hexLocation[100];
    char lenght[100];
    unsigned int hexloca,decleng;
  
    if (s->file_name[0]=='\0')
    {
        fprintf(stderr,"LoadIntoMemory: Error file name is empty.\n");
        return;
    }
  
    int fd=open(s->file_name,O_RDWR);
    if(fd==-1)
    {
        fprintf(stderr,"LoadIntoMemory: Fails to open file: %s \n",s->file_name); 
        return;
    }
  
    printf("Please enter <location> <length>: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", hexLocation, lenght);
    sscanf(hexLocation, "%x", &hexloca);
    sscanf(lenght, "%d", &decleng);
  
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "Debug: file name:%s, location:%x, length:%d ",s->file_name,hexloca,decleng );
    }
  
    lseek(fd, hexloca, SEEK_SET);
    s->mem_count = s->unit_size * decleng;
    read(fd, s->mem_buf, s->unit_size * decleng);
    close(fd); 
  
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "Debug: file name:%s, location:%x, length:%d ",s->file_name,hexloca,decleng );
    }
}

void ToggleDisplayMode(state* s)
{
    char displayFlag=s->display_flag;
  
    if (displayFlag=='0') 
    {
        s->display_flag='1';
        printf("Display Flag now on, hexadecimal representation");
    }
    else if (displayFlag=='1')
    {
        s->display_flag='0';
        printf("Display Flag now off, decimal representation");
    }
}

void MemoryDisplay(state* s)
{
    char input[100];
    char address[100];
    char unit[100];
    unsigned int addressHex=0,unitdec=0;
    printf("Please enter <address> <length>: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", address, unit);
    sscanf(address, "%x", &addressHex);
    sscanf(unit, "%u", &unitdec);

    if (addressHex ==0)
    {
        //starting from the start of the mem_buf
        addressHex=(unsigned int) s->mem_buf;
    }
  
    size_t num_bytes = unitdec * s->unit_size;
  
    if (s->display_flag=='1')
    {
        printf("Hexadecimal\n===========\n");
    }
    else
    {
        printf("Decimal\n=======\n");
    }
  
    unsigned int val;
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

    for (size_t i = 0; i < num_bytes; i= i + s->unit_size)
    {
        int index=s->unit_size-1;
        if(s->display_flag == '1') 
        {
            if (index== 0)
            {
                val=*((unsigned char*)(addressHex+i));
                char * format =hex_formats[index];
                printf(format,val);
            }
            else if (index== 1)
            {
                val=*((unsigned short*)(addressHex+i));
                char * format =hex_formats[index];
                printf(format,val);
            }
            else if (index== 3)
            {
                val= *((unsigned int*)(addressHex+i));
                char * format =hex_formats[index];
                printf(format,val);
            }
        }
        else 
        {
            if (index== 0)
            {
                val=*((unsigned char*)(addressHex+i));
                char * format =dec_formats[index];
                printf(format,val);
            }
            else if (index== 1)
            {
                val=*((unsigned short*)(addressHex+i));
                char * format =dec_formats[index];
                printf(format,val);
            }
            else if (index== 3)
            {
                val= *((unsigned int*)(addressHex+i));
                char * format =dec_formats[index];
                printf(format,val);
            }         
        }
    }
}

void SaveIntoFile(state* s)
{
    char input[100];
    int sourceAddress,targetLocation,length;
   
    if (strcmp(s->file_name, "") == 0)
    {
        if (s->debug_mode == '1') 
        {
            fprintf(stderr,"SaveIntoFile: no name file \n"); 
        }
        return; 
    }

    int fd = open(s->file_name, O_RDWR);
    if (fd == -1)
    {
        if (s->debug_mode == '1') 
        {
            fprintf(stderr,"SaveIntoFile: Error can't open file. \n"); 
        }
        return;
    }
 
    printf("Please enter<source-address> <target-location> <length>: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %x", &sourceAddress, &targetLocation, &length);

    int currPos = lseek(fd, 0, SEEK_END);
    if (currPos >= targetLocation) 
    {
        lseek(fd, 0, SEEK_SET);
        lseek(fd, targetLocation, SEEK_SET);
        if (sourceAddress != 0) 
        {
            write(fd, &sourceAddress, s->unit_size * length);
        }
        else 
        {
            write(fd, &s->mem_buf, s->unit_size * length);
        }
    }
    else 
    {
        printf("SaveIntoFile: Error target location is greater than the size of the file. \n");
        close(fd); 
        return;
    }

    if (s->debug_mode == '1') 
    { 
        fprintf(stderr,"source address: %x\n", sourceAddress);
        fprintf(stderr,"target location: %x\n", targetLocation);
        fprintf(stderr,"length: %d\n", length);
    }
    close(fd);
}


void MemoryModify(state* s)
{
    char input[100];
    char hexLocation[100];
    char value[100];
    unsigned int hexloca,val;
  
    if (s->file_name[0]=='\0')
    {
        fprintf(stderr,"LoadIntoMemory: Error file name is empty.\n");
        return;
    }
  
    printf("Please enter <location> <val>: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", hexLocation, value);
    sscanf(hexLocation, "%x", &hexloca);
    sscanf(value, "%x", &val);
  
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "Debug: location:0x%x, val:%x ",hexloca,val );
    }
    if (hexloca % s->unit_size != 0) 
    {
        printf("MemoryModify: Error Invalid location. The location must be aligned with the unit size.\n");
        return;
    }
    if (hexloca >= s->unit_size * sizeof(s->mem_buf)) 
    {
        printf("Error: Invalid location. The location exceeds the memory buffer size.\n");
        return;
    }
  
    memcpy(&(s->mem_buf[hexloca]),&val,s->unit_size);
}

void Quit(state* s)
{
    if (s->debug_mode=='1')
    {
        fprintf(stderr, "Debug:quitting\n");
    }
  
    exit(0);
}
//----------------------------------------------------------------------------------//

int main(int argc, char **argv)
{
  int i,c;  
  state *s=malloc(sizeof(state));
  resetS(s);
  char out[10];
  
  struct fun_desc menu[] = { 
      { "Toggle Debug Mode", ToggleDebugMode },
      { "Set File Name ", SetFileName },
      {"Set Unit Size",SetUnitSize},
      {"Load Into Memory",LoadIntoMemory},
      { "Toggle Display Mode", ToggleDisplayMode },
      {"Memory Display",MemoryDisplay},
      {"Save Into File",SaveIntoFile},
      { "Memory Modify", MemoryModify },
      {"Quit",Quit},
      { NULL, NULL } }; 
      
  while (1)
  {
    if (s->debug_mode=='1')
    {
        fprintf(stderr,"\nstruct\n");
        fprintf(stderr,"  file_name: %s\n",s->file_name);
        fprintf(stderr,"  unit_size: %d\n",s->unit_size);
        fprintf(stderr,"  mem_count: %u\n",s->mem_count);
        fprintf(stderr,"\n");
    }

    printf("\nPlease choose a function from the menu:\n");
    
    for( i=0; i < 9;i++)
    {
        printf("%d)",i);
        printf("%s",menu[i].name);
        printf("\n");
    }
    
    printf("Option :");

    if (fgets(out,10,stdin)==NULL)
    {
      return 0;
    }
    if (memcmp(out,"\n",1) !=0 &&memcmp(out,"",1)!=0)
    {
        printf("\n");
        c=atoi(out);
        if (c >=0 && c<=9 )
        {
            menu[c].fun(s);
        }
        else
        {
            fprintf(stderr,"Not within bounds\n");
        }
    }
  }
    free(s);
    return 0;
}
