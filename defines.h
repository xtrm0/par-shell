#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdio.h>
#include <errno.h>

#define TESTMEM(s) if (s == NULL) {\
  fprintf(stderr, "(0x45) Erro detetado: Memory Access Error!");\
  exit(ENOMEM);\
}

#define TESTNULL(s, err) if (!s) {\
  fprintf(stderr, err);\
  exit(-1);\
}

#define M_LOCK(mutex) \
 if (pthread_mutex_lock(mutex)) { \
  perror("Error locking mutex"); \
  exit(EXIT_FAILURE); \
 }

#define M_UNLOCK(mutex) \
 if (pthread_mutex_unlock(mutex)) { \
  perror("Error unlocking mutex"); \
  exit(EXIT_FAILURE); \
 }

#define C_WAIT(condition, mutex) \
 if (pthread_cond_wait(condition, mutex)) { \
  perror("Error waiting on condition"); \
  exit(EXIT_FAILURE); \
 }

#define C_SIGNAL(condition) \
 if (pthread_cond_signal(condition)) { \
  perror("Error signaling on condition"); \
  exit(EXIT_FAILURE); \
 }

#endif
