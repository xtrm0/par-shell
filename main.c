#include "commandlinereader.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define N_ARGS 5
int runningProcesses = 0;

void newProcess(char * const *args);
void exitParShell();
void printExitStatus(int pid, int status);
void showPrompt();


int main(int argc, char ** argv) {//renato.nunes@tecnico.ulisboa.pt
  char *args[N_ARGS];
  int maxProcesses;
  if(argc < 2) {
	printf("Correr %s <numero maximo processos filho>\n", argv[0]);
	return 0;
  }
  
  maxProcesses = atoi(argv[1]);
  while(1) {
    //showPrompt(); //?
    if (readLineArguments(args, N_ARGS) <= 0) continue;
    if (strcmp(args[0],"exit") == 0 ) break;
    if (runningProcesses > maxProcesses) {fprintf(stderr, "O numero de filhos e maior que o permitido\n");break;}
    
    newProcess(args);
  }

  exitParShell();

  return 0;
}

/*
  Creates a new process and runs the program specified by args[0]
*/
void newProcess(char * const *args) {
  int pid = fork();
  if (pid == 0) {
    execv(args[0], args);
    fprintf(stderr, "Erro no carregamento do programa %s\n", args[0]);
    exit(-1);
  }
  else if (pid == -1) { //if fork failed
    perror("Erro na criação do processo-filho:\n");
  }
  else { //fork worked and we are in the parent process
    runningProcesses++;
  }
}

/*
  Gracefully exists parshell
*/
void exitParShell() {
  int i;
  int *returnCodes = NULL;
  int *pids = NULL;
  pids = malloc(sizeof(int) * runningProcesses);
  returnCodes = malloc(sizeof(int) * runningProcesses);
  TESTMEM(pids); //checks if malloc worked
  TESTMEM(returnCodes);

  //wait for all the child processes to finish
  for (i=0; i<runningProcesses; i++) {
    pids[i] = wait(returnCodes + i);
  }


  //output the returnCodes for all child processes
  while(runningProcesses--) {
    printExitStatus(pids[runningProcesses], returnCodes[runningProcesses]);
  }
  free(pids);
  free(returnCodes);
}

/*
  Prints the exit status of a process based on status
*/
void printExitStatus(int pid, int status) {
  printf("Process %d terminated", pid);

  //Caso o processo tenha terminado apos chamar o exit():
  if (WIFEXITED(status)) {
    if (status == 0) {
      printf(" normally with success!\n");
    }
    else {
      printf(" normally returning %d\n", WEXITSTATUS(status));
    }
  }

  //Caso o processo tenha sido terminado por um sinal:
  if (WTERMSIG(status)) {
    printf(" with signal %d (%s)", WTERMSIG(status), strsignal(WTERMSIG(status)));
    #ifdef WCOREDUMP
      if (WCOREDUMP(status)) {
        printf(" (core dumped)");
      }
    #endif
    printf("\n");
  }
}

/*
  Outputs to stdout the current working directory
*/
void showPrompt() {
  char *pwd = getcwd(NULL,42);
  TESTMEM(pwd);
  printf("%s$ ", pwd);
  free(pwd);
}
