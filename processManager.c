#include "processManager.h"

typedef struct PROCESSES {
  int dimension;
  int allocated;
  pthread_mutex_t mutexVector;
  RunningProcess * vRunningProcess;
} vProcesses;
static vProcesses processes;


/*
  Initiates the ProcessManager class
*/
void initProcessManager() {
  pthread_mutex_init(&processes.mutexVector, NULL);
  processes.dimension=0;
  processes.allocated=1;
  processes.vRunningProcess=malloc(sizeof(RunningProcess));
}

void endProcessManager() {
  pthread_mutex_destroy(&processes.mutexVector);
}

/*
  Add a new process to the processes vector
*/
void addProcess(int processId) {
  pthread_mutex_lock(&processes.mutexVector);
  if (processes.dimension == processes.allocated) {
    RunningProcess * x = malloc(2*processes.allocated*sizeof(RunningProcess));
    memcpy(x, processes.vRunningProcess, processes.allocated*sizeof(RunningProcess));
    processes.allocated*=2;
    free(processes.vRunningProcess);
    processes.vRunningProcess = x;
  }
  processes.vRunningProcess[processes.dimension].pid = processId;
  clock_gettime( CLOCK_MONOTONIC, &(processes.vRunningProcess[processes.dimension].startTime));
  processes.vRunningProcess[processes.dimension].running = 1;
  processes.dimension++;
  pthread_mutex_unlock(&processes.mutexVector);
}

/*
  Marks a process in the processes vector as ended
*/
void endProcess(int processId, int status) {
  int i;
  pthread_mutex_lock(&processes.mutexVector);
  for (i=0; i<processes.dimension; i++) {
    if (processes.vRunningProcess[i].pid == processId && processes.vRunningProcess[i].running!=0) break;
  }
  processes.vRunningProcess[i].status = status;
  processes.vRunningProcess[i].running = 0;
  clock_gettime( CLOCK_MONOTONIC, &(processes.vRunningProcess[i].endTime));
  pthread_mutex_unlock(&processes.mutexVector);
}

/*
  Returns the real running time for process i
*/
double getRunningTime(int i) {
  double ans;
  pthread_mutex_lock(&processes.mutexVector);
  ans = ( processes.vRunningProcess[i].endTime.tv_sec - processes.vRunningProcess[i].startTime.tv_sec )
             + (double)( processes.vRunningProcess[i].endTime.tv_nsec - processes.vRunningProcess[i].startTime.tv_nsec )
               / (double)1000000000;
  pthread_mutex_unlock(&processes.mutexVector);
  return ans;
}

/*
  Prints the exit status of a process based on status
*/
void printExitStatus(int i) {
  int pid, status;
  double runningTime;
  pthread_mutex_lock(&processes.mutexVector);
  pid = processes.vRunningProcess[i].pid;
  status = processes.vRunningProcess[i].status;
  pthread_mutex_unlock(&processes.mutexVector);
  runningTime = getRunningTime(i);
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


int getProcessCount() {
  int ans;
  pthread_mutex_lock(&processes.mutexVector);
  ans = processes.dimension;
  pthread_mutex_unlock(&processes.mutexVector);
  return ans;
}
