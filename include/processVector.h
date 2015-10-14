#ifndef __PROCESS_VECTOR_H__
#define __PROCESS_VECTOR_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
typedef struct RUNNING_PROCESS {
  int pid;
  int status;
  struct timespec startTime;
  struct timespec endTime;
} RunningProcess;

typedef struct PROCESSES {
  int dimension;
  int allocated;
  RunningProcess * vRunningProcess;
} vProcesses;

vProcesses processes;

void initProcessManager();
void addProcess(int proccessId);
void endProcess(int proccessId, int status);
void printExitStatus(int pid, int status, double);
int getProcessCount();
int getProcessPid(int);
int getProcessStatus(int);
double getRunningTime(int);

#endif
