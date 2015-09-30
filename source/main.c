#include "commandlinereader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define N_ARGS 5
int runningProcesses = 0;

void newProcess(char * const *args);
void exitParShell();


int main() {
  char *args[N_ARGS];

  while(1) {
    if (readLineArguments(args, N_ARGS)==-1) continue;
    if (strcmp(args[0],"exit")==0) break;
    newProcess(args);
  }

  exitParShell();

  return 0;
}

void newProcess(char * const *args) {
  int pid = fork();
  runningProcesses ++;
  if (pid==0) {
    execv(args[0], args);
    fprintf(stderr, "Erro no carregamento do programa %s\n", args[0]);
    exit(-1);
  }
  else if (pid==-1) {
    runningProcesses --;
    fprintf(stderr, "Erro na criação do processo-filho\n");
  }
}

void exitParShell() {
  int returnCode;
  int pid;
  while(runningProcesses--) {
    pid = wait(&returnCode);
    if (returnCode == 0)
      printf("Process %d terminated with success!\n", pid);
    else
      printf("Process %d terminated with error %d\n", pid, returnCode);
  }
}
