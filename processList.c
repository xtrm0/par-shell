#include "processList.h"

typedef struct PROCESSES {
  pthread_mutex_t mutexList;
  RunningProcess * first;
  RunningProcess * tail;
  FILE * output;
  int iteration;
  double totalTime;
} vProcesses;
static vProcesses processes;


/*
  Initiates the ProcessList class
*/
void initProcessList() {
  char inp[3][200];
  int c;
  int i;
  if (pthread_mutex_init(&processes.mutexList, NULL)) {
    fprintf(stderr, "Could not create ProcessList mutex\n");
  }
  processes.iteration = 0;
  processes.totalTime = 0.0;
  processes.first = NULL;
  processes.output = fopen("log.txt", "a+");
  TESTTRUE(processes.output, "Erro ao abrir ficheiro");
  while ((c=fgetc(processes.output))!=EOF) {
    ungetc(c, processes.output);
    for (i=0; i<3; i++) {
      TESTTRUE(fgets(inp[i], 200, processes.output), "Invalid log file!!!");
    }
    TESTTRUE(sscanf(inp[0], "iteracao %d", &(processes.iteration)), "Ficheiro Invalido");
    processes.iteration += 1;
    TESTTRUE(sscanf(inp[2], "total execution time: %lf s", &(processes.totalTime)), "Ficheiro Invalido");
  }
}

void endProcessList() {
  pthread_mutex_destroy(&processes.mutexList);
  RunningProcess * item, * nextitem;
  item = processes.first;
	while (item != NULL){
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
  fclose(processes.output);
  processes.first = NULL;
  processes.tail = NULL;
}

/*
  Add a new process to the processes vector
*/
void addProcess(int processId, int terminalPid, struct timespec startTime) {
  RunningProcess * item = malloc(sizeof(RunningProcess));
  TESTMEM(item);
  item->pid = processId;
  item->terminalPid = terminalPid;
  item->startTime = startTime;
  item->running = 1;
  item->next = NULL;
  M_LOCK(&processes.mutexList);
  if (processes.tail!=NULL) {
    processes.tail->next = item;
    processes.tail = item;
  } else {
    processes.first = processes.tail = item;
  }
  M_UNLOCK(&processes.mutexList);
}

/*
  Marks a process in the processes vector as ended
*/
void endProcess(int processId, int status, struct timespec endTime) {
  RunningProcess * item;
  double ptime;

  M_LOCK(&processes.mutexList);
  item = processes.first;
  while(item!=NULL) {
    if (item->pid == processId && item->running != 0) break;
    item = item->next;
  }
  if (item != NULL) {
    item->status = status;
    item->running = 0;
    item->endTime = endTime;
  }
  M_UNLOCK(&processes.mutexList);
  //XXX: se esta funcao puder correr em mais que uma tarefa, por os fprints dentro de um mutex
  ptime = getRunningTime(item);
  processes.totalTime += ptime;
  fprintf(processes.output,"iteracao %d\n", processes.iteration);
  fprintf(processes.output,"pid: %d execution time: %.4f s\n", item->pid, ptime);
  fprintf(processes.output,"total execution time: %.4f s\n", processes.totalTime);
  processes.iteration++;
}

/*
  Returns the real running time for process i
*/
double getRunningTime(RunningProcess * item) {
  double ans;
  ans = ( item->endTime.tv_sec - item->startTime.tv_sec )
             + (double)( item->endTime.tv_nsec - item->startTime.tv_nsec )
               / (double)1000000000;
  return ans;
}

/*
  Prints the exit status of a process based on status
*/
void printExitStatus(RunningProcess * item) {
  int pid, status;
  double runningTime;
  pid = item->pid;
  status = item->status;
  runningTime = getRunningTime(item);
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
  printf("It took %.4f seconds\n", runningTime);
}

RunningProcess * getFirstRunningProccess() {
  return processes.first;
}

double getTotalTime() {
  double ret;
  M_LOCK(&processes.mutexList);
  ret = processes.totalTime;
  M_UNLOCK(&processes.mutexList);
  return ret;
}
