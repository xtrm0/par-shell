#ifndef __PROCESS_VECTOR_H__
#define __PROCESS_VECTOR_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

typedef struct RUNNING_PROCESS {
  int running;
  int pid;
  int status;
  struct timespec startTime;
  struct timespec endTime;
  struct RUNNING_PROCESS * next;
} RunningProcess;

void initProcessManager();
void endProcessManager();
void addProcess(int proccessId);
void endProcess(int proccessId, int status);
void printExitStatus(int);
int getProcessCount();
int getProcessPid(int);
int getProcessStatus(int);
double getRunningTime(int);

#endif
