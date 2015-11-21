#include "processList.h"

typedef struct PROCESSES {
  pthread_mutex_t mutexList;
  RunningProcess * first;
  RunningProcess * tail;
  FILE * output;
  int iteration;
  int successProcesses;
  double totalTime;
} vProcesses;
static vProcesses processes;


/*
  Initiates the ProcessList class
*/
void initProcessList() {
  char inp[4][200];
  int c;
  if (pthread_mutex_init(&processes.mutexList, NULL)) {
    fprintf(stderr, "Could not create ProcessList mutex\n");
  }
  processes.iteration = 0;
  processes.successProcesses = 0;
  processes.totalTime = 0.0;
  processes.first = NULL;
  processes.output = fopen("log.txt", "a+");
  TESTNULL(processes.output, "Erro ao abrir ficheiro");
  while ((c=fgetc(processes.output))!=EOF) {
    ungetc(c, processes.output);
    if(!fgets(inp[0], 200, processes.output)) {
      fprintf(stderr, "Invalid log file!!!\n");
      exit(-1);
    }
    if(!fgets(inp[1], 200, processes.output)) {
      fprintf(stderr, "Invalid log file!!!\n");
      exit(-1);
    }
    if(!fgets(inp[2], 200, processes.output)) {
      fprintf(stderr, "Invalid log file!!!\n");
      exit(-1);
    }
    if(!fgets(inp[3], 200, processes.output)) {
      fprintf(stderr, "Invalid log file!!!\n");
      exit(-1);
    }
    TESTNULL(sscanf(inp[0], "iteracao %d", &(processes.iteration)), "Ficheiro Invalido");
    processes.iteration += 1;
    TESTNULL(sscanf(inp[2], "total execution time: %lf s", &(processes.totalTime)), "Ficheiro Invalido");
    TESTNULL(sscanf(inp[3], "successful children: %d", &(processes.successProcesses)), "Ficheiro Invalido");
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
void addProcess(int processId) {
  RunningProcess * item = malloc(sizeof(RunningProcess));
  TESTMEM(item);
  item->pid = processId;
  clock_gettime( CLOCK_MONOTONIC, &(item->startTime));
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
void endProcess(int processId, int status) {
  RunningProcess * item;
  double ptime;
  struct timespec endTime;
  clock_gettime( CLOCK_MONOTONIC, &endTime);

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
    if(item->status == 0) {
      processes.successProcesses++;
    }
  }
  M_UNLOCK(&processes.mutexList);
  //XXX: se esta funcao puder correr em mais que uma tarefa, por os fprints dentro de um mutex
  ptime = getRunningTime(item);
  processes.totalTime += ptime;
  fprintf(processes.output,"iteracao %d\n", processes.iteration);
  fprintf(processes.output,"pid: %d execution time: %.4f s\n", item->pid, ptime);
  fprintf(processes.output,"total execution time: %.4f s\n", processes.totalTime);
  fprintf(processes.output,"successful children: %d\n", processes.successProcesses);
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
