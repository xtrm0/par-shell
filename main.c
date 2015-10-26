#include "processManager.h"
#include "commandlinereader.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

#define MAXPAR 4
#define N_ARGS 5
int runningProcesses=0;
int exitCalled=0;
sem_t semRunningProcesses;
sem_t semFreeSlots;

void newProcess(char * const *args);
void exitParShell();
void showPrompt();
void * processMonitor(void * skip);

int main() {
  pthread_t threadMonitor;
  char *args[N_ARGS];
  sem_init(&semRunningProcesses, 0, 0);
  sem_init(&semFreeSlots, 0, MAXPAR);
  initProcessManager();

  if(pthread_create(&threadMonitor, 0,processMonitor, NULL)!= 0) {
    printf("Erro na criação da tarefa\n");
    exit(-1);
  }
  while(1) {
    //showPrompt(); //?
    if (readLineArguments(args, N_ARGS) <= 0) continue;
    if (strcmp(args[0],"exit") == 0 ) break;
    newProcess(args);
  }

  sem_post(&semRunningProcesses); //fazemos post para deixar o processes monitor processar o comando exit
  exitCalled = 1;


  pthread_join(threadMonitor, NULL);

  //The following function is called after all threads have joined, therefore there aren't used any mutexes
  exitParShell();
  endProcessManager();
  
  sem_destroy(&semRunningProcesses);
  sem_destroy(&semFreeSlots);

  return EXIT_SUCCESS;
}

/*
  Process Monitor thread, for monitor processes running time
*/
void * processMonitor(void * skip) {
  int pid, status;
  int semVal;
  while(1) { 
    sem_wait(&semRunningProcesses);
    
    sem_getvalue(&semRunningProcesses, &semVal);
    if (semVal == 0 && exitCalled) {
      break;
    }

    pid = wait(&status);
	if (pid < 0) {
  	  if (errno == EINTR) continue;
  	  else {
  	   	perror("Error waiting for child.");
  	   	exit (EXIT_FAILURE);
  	  }
  	} 
  	else {
  	  sem_post(&semFreeSlots);
  	}
    endProcess(pid, status);
	
  }
  return NULL;
}

/*
  Creates a new process and runs the program specified by args[0]
*/
void newProcess(char * const *args) {
  int pid;
  sem_wait(&semFreeSlots);
  pid = fork();
  if (pid == 0) {
    execv(args[0], args);
    fprintf(stderr, "Erro no carregamento do programa %s\n", args[0]);
    exit(EXIT_FAILURE);
  }
  else if (pid == -1) { //if fork failed
    perror("Erro na criação do processo-filho:\n");
  }
  else { //fork worked and we are in the parent process
    addProcess(pid);
    sem_post(&semRunningProcesses);
  }
}

/*
  Gracefully exists parshell
*/
void exitParShell() {
  RunningProcess * item = getFirstRunningProccess();
  while (item!=NULL) {
    printExitStatus(item);
    item = item->next;
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
