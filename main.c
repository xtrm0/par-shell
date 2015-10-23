#include "processManager.h"
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
pthread_mutex_t mutexRunningProcesses;

void newProcess(char * const *args);
void exitParShell();
void showPrompt();
void * processMonitor(void * skip);

int main() {
  pthread_t threadMonitor;
  char *args[N_ARGS];
  pthread_mutex_init(&mutexRunningProcesses, NULL);
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


  exitCalled = 1;


  pthread_join(threadMonitor, NULL);

  //The following function is called after all threads have joined, therefore there aren't used any mutexes
  exitParShell();
  endProcessManager();
  pthread_mutex_destroy(&mutexRunningProcesses);

  return EXIT_SUCCESS;
}

/*
  Process Monitor thread, for monitor processes running time
*/
void * processMonitor(void * skip) {
  int pid, status;
  while(1) {
    pthread_mutex_lock(&mutexRunningProcesses);
    if (runningProcesses > 0) {
      pthread_mutex_unlock(&mutexRunningProcesses);
      pid = wait(&status);
  	  if (pid < 0) {
  	  	if (errno == EINTR) continue;
  	  	else {
  	   	 perror("Error waiting for child.");
  	   	 exit (EXIT_FAILURE);
  	  	}
  	  }
      endProcess(pid, status);
      pthread_mutex_lock(&mutexRunningProcesses);
      runningProcesses--;
      pthread_mutex_unlock(&mutexRunningProcesses);
    }
    else  {
  		pthread_mutex_unlock(&mutexRunningProcesses);
    }
    pthread_mutex_lock(&mutexRunningProcesses);
    if (runningProcesses == 0) {
      pthread_mutex_unlock(&mutexRunningProcesses);
      sleep(1);
    }
    else {
   		pthread_mutex_unlock(&mutexRunningProcesses);
    }

    pthread_mutex_lock(&mutexRunningProcesses);
    if (runningProcesses == 0 && exitCalled) {
      pthread_mutex_unlock(&mutexRunningProcesses);
      break;
    }
    else {
      pthread_mutex_unlock(&mutexRunningProcesses);
    }
  }
  return NULL;
}

/*
  Creates a new process and runs the program specified by args[0]
*/
void newProcess(char * const *args) {
  int pid = fork();
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
    pthread_mutex_lock(&mutexRunningProcesses);
    runningProcesses++;
    pthread_mutex_unlock(&mutexRunningProcesses);
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
