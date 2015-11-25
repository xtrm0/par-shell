#include "processList.h"
#include "commandlinereader.h"
#include "terminalList.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXPAR 4
#define N_ARGS 5
#define BUFFER_LEN PIPE_BUF
#define INPUT_FILE "par-shell-in"
int runningProcesses=0;
int processesWaitingToRun=0;
int exitCalled=0;
int outPipe=-1;
pthread_mutex_t mutexRunningProcesses;
pthread_cond_t condRunningProcesses;
pthread_cond_t condFreeSlots;
void newProcess(char * const *args, int terminalPid);
void exitParShell();
void showPrompt();
void * processMonitor(void * skip);

int readFromPipe(int inputPipe, char * buffer, int * terminalPid, TerminalList * termlist) {
  printf("Running readFromPipe\n");
  int len;
  int op;
  TESTTRUE((read(inputPipe, &op, sizeof(int))==sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
  printf("op: %d\n", op);
  if (op==0) {
    TESTTRUE((read(inputPipe, &len, sizeof(int)) == sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
    printf("len: %d\n", len);
    TESTTRUE((read(inputPipe, terminalPid, sizeof(int)) == sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
    printf("pid: %d\n", *terminalPid);
    TESTTRUE((read(inputPipe, buffer, len) == len), "Erro no formato do pipe (" _AT_ ")\n");
    buffer[len]=0;
    return 0;
  } else if (op==1) { //informacao que foi criado um novo terminal
    TESTTRUE((read(inputPipe, terminalPid, sizeof(int)) == sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
    lst_insert(termlist, *terminalPid);
    printf("pid: %d\n", *terminalPid);
    return 1;
  } else if (op==2) { //informacao que foi fechado um terminal
    TESTTRUE((read(inputPipe, terminalPid, sizeof(int)) == sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
    lst_remove(termlist, *terminalPid);
    printf("pid: %d\n", *terminalPid);
    return 2;
  } else if (op==3) { //informacao que e para se fechar a par-shell
    exitCalled = 1;
    printf("RECEIVED EXIT SIGNAL!\n");
    return 3;
  }
  fprintf(stderr,"Unknown operator!\n");
  return -1;
}

void handleSIGINT(int signum) {
  if (signum==SIGINT) {
    signal(SIGINT,SIG_IGN);
    if (outPipe == -1) {
      fprintf(stderr, "SIGINT received during startup, exiting\n");
      exit(-1);
    }
    int mode = 3;
    char * buffer = (char*)&mode;
    write(outPipe, buffer, sizeof(int));
  }
}


int main() {
  int inputPipe;
  char buffer[BUFFER_LEN];
  int ret;
  int terminalPid;
  TerminalList * termlist = lst_new();
  pthread_t threadMonitor;
  char *args[N_ARGS];
  struct stat st;
  signal(SIGINT, handleSIGINT);
  if (pthread_mutex_init(&mutexRunningProcesses, NULL)) {
    fprintf(stderr, "Could not create runningProcesses mutex\n");
    exit(-1);
  }
  if (pthread_cond_init(&condRunningProcesses, NULL)) {
    fprintf(stderr, "Could not create runningProcesses cond\n");
    exit(-1);
  }
  if (pthread_cond_init(&condFreeSlots, NULL)) {
    fprintf(stderr, "Could not create FreeSlots cond\n");
    exit(-1);
  }
  unlink("par-shell-in");
  if (stat("par-shell-in", &st) == 0) {
    fprintf(stderr, "Parshell is already running on this directory!!!\n");
    exit(-1);
  }
  if (mkfifo(INPUT_FILE, 0777)<0) {
    fprintf(stderr, "Could not create fifo " INPUT_FILE "\n");
    exit(-1);
  }
  printf("A abrir o pipe\n");
  if ((inputPipe = open(INPUT_FILE, O_RDONLY)) < 0) {
    fprintf(stderr, "Could not create fifo " INPUT_FILE "\n");
    exit(-1);
  }
  printf("Pipe aberto!!!\n");
  if ((outPipe = open(INPUT_FILE, O_WRONLY) < 0)) {
    fprintf(stderr, "Erro ao abrir o ficheiro de output" INPUT_FILE "\n");
    exit(-1);
  }
  initProcessList();
  if(pthread_create(&threadMonitor, 0,processMonitor, NULL)!= 0) {
    printf("Erro na criação da tarefa\n");
    exit(-1);
  }

  while(1) {
    if (exitCalled) break;
    if (readFromPipe(inputPipe, buffer, &terminalPid, termlist)!=0) continue;
    printf("Comando: %s\n", buffer);
    ret = readLineArguments(args, N_ARGS, buffer, BUFFER_LEN);
    if (!ret) continue;
    if (strcmp(args[0],"stats") == 0) {
      //XXX:TODO
      continue;
    }
    if (strcmp(args[0],"exit-global") == 0) {
      int mode = 3;
      char * opt = (char*)&mode;
      write(outPipe, opt, sizeof(int));
      printf("Writing exit mode to pipe\n");
      continue;
    }
    processesWaitingToRun++;
    newProcess(args, terminalPid);
  }

  //Mata todos os processos de terminal
  lst_destroy(termlist);

  C_SIGNAL(&condRunningProcesses);

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
    M_LOCK(&mutexRunningProcesses);
    while(runningProcesses==0) {
      if (exitCalled && processesWaitingToRun==0) {
        pthread_exit(NULL);
      }
      C_WAIT(&condRunningProcesses, &mutexRunningProcesses);
    }

    M_UNLOCK(&mutexRunningProcesses);

    pid = wait(&status);
	  if (pid < 0) {
  	  if (errno == EINTR) continue;
  	  else {
  	   	perror("Error waiting for child");
  	   	exit (EXIT_FAILURE);
  	  }
  	}
  	else {
      M_LOCK(&mutexRunningProcesses);
      runningProcesses--;
      C_SIGNAL(&condFreeSlots);
      M_UNLOCK(&mutexRunningProcesses);
  	}
    endProcess(pid, status);
  }
}

/*
  Creates a new process and runs the program specified by args[0]
*/
void newProcess(char * const *args, int terminalPid) {
  int pid;
  M_LOCK(&mutexRunningProcesses);
  while(runningProcesses==MAXPAR) {
    C_WAIT(&condFreeSlots, &mutexRunningProcesses);
  }
  M_UNLOCK(&mutexRunningProcesses);
  pid = fork();
  if (pid == 0) {
    FILE * outFile;
    char outFileName[30];
    sprintf(outFileName, "par-shell-out-%d", getpid());
    unlink(outFileName);
    fprintf(stderr, "%s\n", outFileName);
    outFile = fopen(outFileName, "w");
    printf("outfile: %d\n", fileno(outFile));
    fflush(stdout);
    printf("dup2 return: %d\n", dup2(fileno(outFile), STDOUT_FILENO));
    fclose(outFile);
    execv(args[0], args);
    fprintf(stderr, "Erro no carregamento do programa %s\n", args[0]);
    exit(EXIT_FAILURE);
  }
  else if (pid == -1) { //if fork failed
    M_LOCK(&mutexRunningProcesses);
    processesWaitingToRun--;
    C_SIGNAL(&condRunningProcesses);
    M_UNLOCK(&mutexRunningProcesses);
    perror("Erro na criação do processo-filho:\n");
  }
  else { //fork worked and we are in the parent process
    addProcess(pid, terminalPid);
    M_LOCK(&mutexRunningProcesses);
    runningProcesses++;
    processesWaitingToRun--;
    C_SIGNAL(&condRunningProcesses);
    M_UNLOCK(&mutexRunningProcesses);
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
