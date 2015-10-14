#include "processVector.h"
#include "commandlinereader.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#define N_ARGS 5
int runningProcesses=0;
int exitCalled=0;

void newProcess(char * const *args);
void exitParShell();
void showPrompt();

void * processMonitor(void * skip) {
  int pid, status;
  while(1) {
    //bloqueia mutex
    if (runningProcesses > 0) {
      //desbloqueia mutex
      pid = wait(&status);
      //bloqueia mutex
      endProcess(pid, status);
      runningProcesses--;
    }
    //desbloqueia mutex
    //bloqueia
    if (runningProcesses == 0) {
      //desbloqueia
      sleep(1);
    }
    else {
      //desbloqueia
    }

    //bloqueia
    //bloqueia2
    if (runningProcesses == 0 && exitCalled) {
      //desbloqueia
      break;
    }
    else {
      //desbloqueia
    }
  }
  return NULL;
}

int main() {
  pthread_t threadMonitor;
  char *args[N_ARGS];

  if(pthread_create (&threadMonitor, 0,processMonitor, NULL)!= 0) {
    printf("Erro na criação da tarefa\n");
    exit(-1);
  }

  while(1) {
    //showPrompt(); //?
    if (readLineArguments(args, N_ARGS) <= 0) continue;
    if (strcmp(args[0],"exit") == 0) break;
    newProcess(args);
  }

  //bloqueia mutex
  exitCalled = 1;
  //desbloqueia mutex

  pthread_join(threadMonitor, NULL);
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
    //bloqueia mutex
    addProcess(pid);
    runningProcesses++;
    //desbloqueia mutex
  }
}

/*
  Gracefully exists parshell
*/
void exitParShell() {
  int i;
  int n = getProcessCount();
  //output the returnCodes for all child processes
  for (i=0; i<n; i++) {
    printExitStatus(getProcessPid(i), getProcessStatus(i), getRunningTime(i));
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
