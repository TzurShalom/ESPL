#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus; 

typedef struct link link;

struct link {
link *nextVirus;
virus *vir;
};

virus* readVirus(FILE* file)
{
    virus * v = malloc(sizeof(virus));
    if (fread(v,sizeof(char),18,file) != 18) {free(v); return NULL;}
    v->sig = malloc(v->SigSize);
    fread(v->sig,sizeof(char),v->SigSize,file);
    return v;
}

void printVirus(virus* virus, FILE* output)
{
    fprintf(output,"Virus name: %s\n",virus->virusName);
    fprintf(output,"Virus size: %d\n",virus->SigSize);
    fprintf(output,"signature:\n");

    for (int i = 0; i < virus->SigSize; i++)
    {
        fprintf(output,"%02X ",*(virus->sig + i));
    }

    fprintf(output,"\n\n");
}

void list_print(link* virus_list, FILE* output)
{
    link * link = virus_list;

    while (link != NULL)
    {
        printVirus(link->vir,output);  
        link = link->nextVirus; 
    }
}

void print_viruses(char *fileName, link **virus_list_ptr)
{
   list_print((*virus_list_ptr),stdout);
}

link* list_append(link* virus_list, virus* data)
{
    link * link = malloc(sizeof(link));
    link->vir = data;
    link->nextVirus = NULL;

    if (virus_list != NULL) 
        {link->nextVirus = virus_list;}

    return link;
}

void list_free(link *virus_list)
{
    link * link = NULL;

    while (virus_list != NULL)
    {
        link = virus_list;
        virus_list = virus_list->nextVirus;

        free(link->vir->sig);
        free(link->vir);
        free(link);
    }  
}

void free_viruses(char *fileName, link **virus_list_ptr)
{
    list_free(*virus_list_ptr);
    printf("\nDONE!\n");
    exit(0);
}

void file_format_test(char *fileName)
{
    FILE * input = fopen(fileName,"r+");

    char format[4];
    fread(format,sizeof(char),sizeof(format),input);

    fclose(input);

    if (strcmp(format,"VISL") != 0) {printf("The magic number is incorrect\n"); exit(1);}
}

void load_viruses_to_list(char *fileName, link **virus_list_ptr)
{
    file_format_test(fileName);

    FILE * input = fopen(fileName,"r+");
    fseek(input,4,SEEK_SET);

    virus * v;

    while ((v = readVirus(input)) != NULL)
    {
        (*virus_list_ptr) = list_append((*virus_list_ptr),v);
    }

    fclose(input);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) 
{
    link * link = virus_list;
    while (link != NULL)
    {
        for (int i = 0; i < size ; i++)
        {       
            if (memcmp(buffer + i,link->vir->sig,link->vir->SigSize) == 0)
            {
                printf("The starting byte location in the suspected file: %i\n",i);
                printf("The virus name: %s\n",link->vir->virusName);
                printf("The size of the virus signature: %i\n\n",link->vir->SigSize);
            }
        }
        link = link->nextVirus; 
    }
}

void detect_viruses(char *fileName, link **virus_list_ptr)
{
    FILE * input = fopen(fileName,"r+");
    char *buffer = malloc(10000);
    int length = fread(buffer,sizeof(char),10000,input);
    fclose(input);
    detect_virus(buffer,length,(*virus_list_ptr)); 
    free(buffer);
    
}

void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE * input = fopen(fileName,"r+");
    fseek(input,signatureOffset,SEEK_SET);

    char c[] = {0xC3};
    fwrite(c,sizeof(char),sizeof(c),input);

    fclose(input);
}

void neutralize_viruses(char *fileName, link **virus_list_ptr)
{
    FILE * input = fopen(fileName,"r+");
    char *buffer = malloc(10000);
    int length = fread(buffer,sizeof(char),10000,input);
    fclose(input);
    link * link = (*virus_list_ptr);
    while (link != NULL)
    {
        for (int i = 0; i < length; i++)
        {
            if (memcmp(buffer + i,link->vir->sig,link->vir->SigSize) == 0)
            {
                neutralize_virus(fileName,i);
            }
        }
        link = link->nextVirus; 
    }
    free(buffer);
}

struct fun_desc 
{
  char *name;
  void (*fun)(char *, link **);
}; 

int main(int argc, char **argv)
{
    struct fun_desc menu[] = { { "Load signatures", load_viruses_to_list }, { "Print signatures", print_viruses }, { "Detect viruses", detect_viruses } , { "Fix file", neutralize_viruses } , { "Quit", free_viruses } , { NULL, NULL } }; 
    int bound = sizeof(menu) / sizeof(struct fun_desc)-1;
    printf("\nPlease choose a function (ctrl^D for exit):\n");
    for (int i = 1; i <=bound; i++) {printf("%i) %s\n",i,menu[i-1].name);}
    printf("Option : ");
    
    link * link = NULL;

    char c[4];
    int i;
    char fileName[256];
    while (fgets(c,4,stdin))
    {
        i = atoi(c);
        if (!((1 <= i) & (i <= bound))) { 
            exit(1);
        }

        if ( i==1 )
        {
            printf("Type a file name: ");
            fgets(fileName,256,stdin);
            sscanf(fileName,"%s",fileName);
        }
        if (i==3 || i==4 ){
            menu[i-1].fun(argv[1],&link);  
        }
        else {
            menu[i-1].fun(fileName,&link);
        }
      
        printf("\nDONE!\n");

        printf("\nPlease choose a function (ctrl^D for exit):\n");
        for (int i = 1; i <= bound; i++) {printf("%i) %s\n",i,menu[i-1].name);}
        printf("Option : ");
    }

    return 0;
}
