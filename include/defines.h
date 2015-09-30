#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdio.h>
#include <errno.h>

#define LIBERATE(x) if (x!=NULL) free(x);

#define TESTMEM(s) if (s == NULL) {\
  fprintf(stderr, "(0x45) Erro detetado: Memory Access Error!");\
  exit(ENOMEM);\
}

#endif
