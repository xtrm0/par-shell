#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER_LEN PIPE_BUF
#define INPUT_FILE "par-shell-in"
int outfileid;

///TODO: TESTAR SE O PIPE EXISTE
///TODO: TESTAR STUFF
///TODO: TESTAR OS WRITE (SE DEU ERRO)

int main(int argc, char** argv) {
  char * c;
  char buffer[BUFFER_LEN];
  int len, pid, mode;
  char * orig;
  pid = getpid();
  if (argc!=2) {
    fprintf(stderr, "Modo de utilizacao: %s <pathname para o pipe>", argv[0]);
    exit(-1);
  }
  outfileid = open(argv[1], O_WRONLY);
  mode = 1;
  orig = (char*)&mode;
  memcpy(buffer, orig, sizeof(int));
  orig = (char*)&pid;
  memcpy(buffer+sizeof(int), orig, sizeof(int));
  write(outfileid, buffer, 2*sizeof(int));
  while ((c = fgets(buffer + 3*sizeof(int), BUFFER_LEN - 3*sizeof(int), stdin)) > 0) {
    printf("\"%s\"\n", buffer + 3*sizeof(int));
    if (c == NULL || *c == EOF) {
      break;
    }
    if (strcmp("exit-global\n", buffer + 3*sizeof(int))==0) {
      mode = 3;
      orig = (char*)&mode;
      memcpy(buffer, orig, sizeof(int));
      write(outfileid, buffer, sizeof(int));
    }
    len = strlen(buffer+3*sizeof(int));
    len++;
    mode = 0;
    orig = (char*)&mode;
    memcpy(buffer, orig, sizeof(int));
    orig = (char*)&len;
    memcpy(buffer+sizeof(int), orig, sizeof(int));
    orig = (char*)&pid;
    memcpy(buffer+2*sizeof(int), orig, sizeof(int));
    write(outfileid, buffer, 3*sizeof(int) + len);
  }
  mode = 2;
  orig = (char*)&mode;
  memcpy(buffer, orig, sizeof(int));
  orig = (char*)&pid;
  memcpy(buffer+sizeof(int), orig, sizeof(int));
  write(outfileid, buffer, 2*sizeof(int));
  //close(outfileid);
}
