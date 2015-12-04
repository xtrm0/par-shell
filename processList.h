#ifndef __PROCESS_VECTOR_H__
#define __PROCESS_VECTOR_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "defines.h"

typedef struct RUNNING_PROCESS {
  int running;
  int pid;
  int status;
  int terminalPid;
  struct timespec startTime;
  struct timespec endTime;
  struct RUNNING_PROCESS * next;
} RunningProcess;

void initProcessList();
void endProcessList();
void addProcess(int proccessId, int terminalPid, struct timespec startTime);
void endProcess(int proccessId, int status, struct timespec endTime);
void printExitStatus(RunningProcess *);
double getRunningTime(RunningProcess *);
RunningProcess * getFirstRunningProccess();
double getTotalTime();

#endif
