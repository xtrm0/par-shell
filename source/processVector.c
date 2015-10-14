#include "processVector.h"

void initProcessManager() {
  processes.dimension=0;
  processes.allocated=0;
  processes.vRunningProcess=NULL;
}

void addProcess(int proccessId) {
  if (processes.dimension == processes.allocated) {
    RunningProcess * x = malloc(2*processes.dimension*sizeof(RunningProcess));
    memcpy(x, processes.vRunningProcess, processes.dimension*sizeof(RunningProcess));
    free(processes.vRunningProcess);
    processes.vRunningProcess = x;
  }
  processes.vRunningProcess[processes.dimension].pid = proccessId;
  clock_gettime( CLOCK_REALTIME, &processes.vRunningProcess[processes.dimension].startTime);
  processes.dimension++;
}

void endProcess(int proccessId, int status) {
  int i;
  for (i=0; i<processes.dimension; i++) {
    if (processes.vRunningProcess[i].pid == proccessId) break;
  }
  processes.vRunningProcess[i].status = status;
  clock_gettime( CLOCK_REALTIME, &processes.vRunningProcess[i].endTime);
}

double getRunningTime(int i) {
  return ( processes.vRunningProcess[i].endTime.tv_sec - processes.vRunningProcess[i].startTime.tv_sec )
             + (double)( processes.vRunningProcess[i].endTime.tv_nsec - processes.vRunningProcess[i].startTime.tv_nsec )
               / (double)1000000000;
}

/*
  Prints the exit status of a process based on status
*/
void printExitStatus(int pid, int status, double runningTime) {
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
  printf("It took %f seconds\n", runningTime);
}

int getProcessCount() {
  return processes.dimension;
}
int getProcessPid(int i) {
  return processes.vRunningProcess[i].pid;
}
int getProcessStatus(int i) {
  return processes.vRunningProcess[i].status;
}
