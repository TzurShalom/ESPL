#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* map(char *array, int array_length, char (*f) (char))
{
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for (int i = 0; i < array_length; i++)
  {
    mapped_array[i] = (*f) (array[i]);
  }
  return mapped_array;
}

char f (char c)
{
  return c+1;
}

char my_get(char c) /* Ignores c, reads and returns a character from stdin using fgetc. */
{
  return getc(stdin);
}

char cprt(char c) /* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
{
  if ((0x20 <= c) & (c <= 0x7E)) {printf("%c\n",c);}
  else {printf(".\n");}
  return c;
}

char encrypt(char c) /* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
{
  if ((0x20 <= c) & (c <= 0x7E)) {return c+1;}
  else {return c;}
}

char decrypt(char c) /* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
{
  if ((0x20 <= c) & (c <= 0x7E)) {return c-1;}
  else {return c;}
}

char xprt(char c) /* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */ 
{
  printf("%x\n",c);
  return c;
}

struct fun_desc 
{
  char *name;
  char (*fun)(char);
}; 

int main(int argc, char **argv)
{
  /* TODO: Test your code */

  // char array[] = {'a','b','c'};
  // char* array2Ptr = array; 

  // int array_length = 3;
  // char (*f_ptr) (char) = f; 

  // char *mapped_array = map(array2Ptr,array_length,f_ptr);
  
  // for (int i = 0; i < array_length; i++)
  // {
  //   printf("%c\n",mapped_array[i]);
  // }

  // free(mapped_array);

  // int base_len = 5;
  // char arr1[base_len];
  // char* arr2 = map(arr1, base_len, my_get);
  // char* arr3 = map(arr2, base_len, cprt);
  // char* arr4 = map(arr3, base_len, xprt);
  // char* arr5 = map(arr4, base_len, encrypt);
  // char* arr6 = map(arr5, base_len, decrypt);
  // free(arr2);
  // free(arr3);
  // free(arr4);
  // free(arr5);
  // free(arr6); 

  char c[10];
  int i;

  char *carray = malloc( sizeof(char) * 5);

  struct fun_desc menu[] = { { "Get string", my_get }, { "Print string", cprt }, { "Encrypt", encrypt } , { "Decrypt", decrypt } , { "Print Hex", xprt } , { NULL, NULL } }; 
  int bound = sizeof(menu) / sizeof(struct fun_desc) - 1;

  printf("Please choose a function (ctrl^D for exit):\n");
  for (int i = 0; i < bound; i++) {printf("%i) %s\n",i,menu[i].name);}
  printf("Option : ");

  while (fgets(c,10,stdin))
  {
    i = atoi(c);
    if ((0 <= i) & (i < bound)) {printf("\nWithin bounds\n");}
    else {printf("\nNot within bounds\n"); return 1;}

    carray = map (carray, 5, menu[i].fun);
    printf("DONE.\n\n");

    printf("Please choose a function (ctrl^D for exit):\n");
    for (int i = 0; i < bound; i++) {printf("%i) %s\n",i,menu[i].name);}
    printf("Option : ");
  }
  free(carray);
  return 0;
}
