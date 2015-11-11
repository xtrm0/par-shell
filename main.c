#include "processList.h"
#include "commandlinereader.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define MAXPAR 4
#define N_ARGS 5
#define BUFFER_LEN 200
int runningProcesses=0;
int processesWaitingToRun=0;
int exitCalled=0;
pthread_mutex_t mutexRunningProcesses;
pthread_cond_t condRunningProcesses;
pthread_cond_t condFreeSlots;
void newProcess(char * const *args);
void exitParShell();
void showPrompt();
void * processMonitor(void * skip);

int main() {
  char buffer[BUFFER_LEN];
  int ret;
  pthread_t threadMonitor;
  char *args[N_ARGS];
  if (pthread_mutex_init(&mutexRunningProcesses, NULL)) {
    fprintf(stderr, "Could not create runningProcesses mutex\n");
  }
  if (pthread_cond_init(&condRunningProcesses, NULL)) {
    fprintf(stderr, "Could not create runningProcesses cond\n");
  }
  if (pthread_cond_init(&condFreeSlots, NULL)) {
    fprintf(stderr, "Could not create FreeSlots cond\n");
  }
  initProcessList();

  if(pthread_create(&threadMonitor, 0,processMonitor, NULL)!= 0) {
    printf("Erro na criação da tarefa\n");
    exit(-1);
  }
  while(1) {
    //showPrompt(); //?
    ret = readLineArguments(args, N_ARGS, buffer, BUFFER_LEN);
    if (!ret) continue;
    if (strcmp(args[0],"exit") == 0 || !~ret) break;
    processesWaitingToRun++;
    newProcess(args);
  }

  exitCalled = 1;
  pthread_cond_signal(&condRunningProcesses);

  pthread_join(threadMonitor, NULL);

  //The following function is called after all threads have joined, therefore there aren't used any mutexes
  exitParShell();
  endProcessList();

  pthread_mutex_destroy(&mutexRunningProcesses);
  pthread_cond_destroy(&condRunningProcesses);
  pthread_cond_destroy(&condFreeSlots);

  return EXIT_SUCCESS;
}

/*
  Process Monitor thread, for monitoring processes running time
*/
void * processMonitor(void * skip) {
  int pid, status;
  while(1) {
    pthread_mutex_lock(&mutexRunningProcesses);
    while(runningProcesses==0) {
      if (exitCalled && processesWaitingToRun==0) {
        goto endProcessMonitorThread;
      }
      pthread_cond_wait(&condRunningProcesses, &mutexRunningProcesses);
    }

    pthread_mutex_unlock(&mutexRunningProcesses);

    pid = wait(&status);
	  if (pid < 0) {
  	  if (errno == EINTR) continue;
  	  else {
  	   	perror("Error waiting for child");
  	   	exit (EXIT_FAILURE);
  	  }
  	}
  	else {
      pthread_mutex_lock(&mutexRunningProcesses);
      runningProcesses--;
      pthread_cond_signal(&condFreeSlots);
      pthread_mutex_unlock(&mutexRunningProcesses);
  	}
    endProcess(pid, status);
  }
  endProcessMonitorThread:
  pthread_exit(NULL);
}

/*
  Creates a new process and runs the program specified by args[0]
*/
void newProcess(char * const *args) {
  int pid;
  pthread_mutex_lock(&mutexRunningProcesses);
  while(runningProcesses==MAXPAR) {
    pthread_cond_wait(&condFreeSlots, &mutexRunningProcesses);
  }
  pthread_mutex_unlock(&mutexRunningProcesses);

  pid = fork();
  if (pid == 0) {
    execv(args[0], args);
    fprintf(stderr, "Erro no carregamento do programa %s\n", args[0]);
    exit(EXIT_FAILURE);
  }
  else if (pid == -1) { //if fork failed
    pthread_mutex_lock(&mutexRunningProcesses);
    processesWaitingToRun--;
    pthread_cond_signal(&condRunningProcesses);
    pthread_mutex_unlock(&mutexRunningProcesses);
    perror("Erro na criação do processo-filho:\n");
  }
  else { //fork worked and we are in the parent process
    addProcess(pid);
    pthread_mutex_lock(&mutexRunningProcesses);
    runningProcesses++;
    processesWaitingToRun--;
    pthread_cond_signal(&condRunningProcesses);
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
