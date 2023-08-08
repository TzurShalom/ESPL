#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    int debugFlag = 0;
    int keyFlag = 0;
    char * key;
    FILE * inputFile = stdin;
    FILE * outputFile = stdout;
    FILE * errorFile = stderr;

    for (int i = 1; i < argc; i++)
    {
      if (strncmp(argv[i],"+D",2) == 0) {debugFlag = 1;} 
      else if (strncmp(argv[i],"-D",2) == 0) {debugFlag = 0;} 
      else if ((strncmp(argv[i],"-e",2) == 0) | (strncmp(argv[i],"+e",2) == 0))
      {
        if (debugFlag) {fprintf(errorFile,"%s\n",argv[i]);}
        key = argv[i];
        keyFlag = 1;
      }
      else if ((argv[i][0] == '-') & (argv[i][1] == 'i'))
      {
        if (debugFlag) {fprintf(errorFile,"%s\n",argv[i]);}
          
        inputFile = fopen(argv[i]+2,"r");
     
        if (inputFile == NULL)
        {
          fprintf(errorFile,"There is an error opening the given file\n");
          return 1;
        }     
      } 
      else if ((argv[i][0] == '-') & (argv[i][1] == 'o'))
      {
        if (debugFlag) {fprintf(errorFile,"%s\n",argv[i]);}
          
        outputFile = fopen(argv[i]+2,"w");
      
        if (outputFile == NULL)
        {
          fprintf(errorFile,"There is an error opening the given file\n");
          return 1;
        }   
      }
    }

    char a,b,c;
    int index = 2;
    int add = 1;

    if (key[0] == '-') {add = 0;}

    while ((c = fgetc(inputFile)) != EOF) 
    {
      if (keyFlag)
      {
        if (key[index] == 0){index = 2;}
        b = key[index] - '0';

        if (('0' <= c) & (c <= '9'))
        {
          a = c - '0';
          if (add) {c = (((a + b) % 10) + '0');}
          else {c = (((10 + a - b) % 10) + '0');}
        } 
        if (('a' <= c) & (c <= 'z')) 
        {
          a = c - 'a';
          if (add) {c = ((a + b) % 26) + 'a';}
          else {c = ((26 + a - b) % 26) + 'a';}
        }     
        else if (('A' <= c) & (c <= 'Z'))
        {
          a = c - 'A';
          if (add) {c = ((a + b) % 26) + 'A';}
          else {c = (((26 + a - b) % 26) + 'A');}
        }
        index++;
      }
      fputc(c,outputFile);
    }

    if (inputFile != stdin) {fclose(inputFile);}
    if (outputFile != stdout) {fclose(outputFile);}

    return 0;
}
