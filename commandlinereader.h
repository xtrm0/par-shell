//
// Command line reader (header file), version 1
// Sistemas Operativos, DEI/IST/ULisboa 2015-16
//

#ifndef COMMANDLINEREADER_H
#define COMMANDLINEREADER_H
#endif

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

int readLineArguments(char **argVector, int vectorSize);
