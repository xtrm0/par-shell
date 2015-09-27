//
// Command line reader (implementation), version 1
// Sistemas Operativos, DEI/IST/ULisboa 2015-16
//

#include <string.h>
#include <stdio.h>

/* 
Reads up to 'vectorSize' space-separated arguments from the standard input
and saves them in the entries of the 'argVector' argument.
This function returns once enough arguments are read or the end of the line 
is reached

Arguments: 
 'argVector' should be a vector of char* previously allocated with
 as many entries as 'vectorSize'
 'vectorSize' is the maximum number of arguments that should be read

Return value:
 The number of arguments that were read, or -1 if some error occurred.
*/

int readLineArguments(char **argVector, int vectorSize)
{
  int next_arg = 0;
  int numtokens = 0;
  char *s = " \n\t";

  char *str = NULL;
  size_t size = 0;
  int i;

  char *token;

  if (argVector == NULL || vectorSize == 0)
    return 0;

  if (getline(&str, &size, stdin) < 0) {
    printf("!!\n");
    return -1;
  }
   
  /* get the first token */
  token = strtok(str, s);
   
  /* walk through other tokens */
  while( numtokens < vectorSize-1 && token != NULL ) {
    
    printf( "encontrou %s\n", token );
    
    argVector[numtokens] = token;
    numtokens ++;
    
    token = strtok(NULL, s);
  }
   
  for (i = numtokens; i<vectorSize; i++) {
    argVector[i] = NULL;
  }
   
  return numtokens;
}

