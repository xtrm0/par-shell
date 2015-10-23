#include "processManager.h"

typedef struct PROCESSES {
  pthread_mutex_t mutexList;
  RunningProcess * first;
  RunningProcess * tail;
} vProcesses;
static vProcesses processes;


/*
  Initiates the ProcessManager class
*/
void initProcessManager() {
  pthread_mutex_init(&processes.mutexList, NULL);
  processes.first = NULL;
}

void endProcessManager() {
  pthread_mutex_destroy(&processes.mutexList);
  RunningProcess * item, * nextitem;
  item = processes.first;
	while (item != NULL){
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
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
  pthread_mutex_lock(&processes.mutexList);
  if (processes.tail!=NULL) {
    processes.tail->next = item;
    processes.tail = item;
  } else {
    processes.first = processes.tail = item;
  }
  pthread_mutex_unlock(&processes.mutexList);
}

/*
  Marks a process in the processes vector as ended
*/
void endProcess(int processId, int status) {
  RunningProcess * item;
  pthread_mutex_lock(&processes.mutexList);
  item = processes.first;
  while(item!=NULL) {
    if (item->pid == processId && item->running != 0) break;
    item = item->next;
  }
  if (item != NULL) {
    item->status = status;
    item->running = 0;
    clock_gettime( CLOCK_MONOTONIC, &(item->endTime));
  }
  pthread_mutex_unlock(&processes.mutexList);
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
