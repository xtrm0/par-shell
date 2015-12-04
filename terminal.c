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
#define MKTEMP_TEMPLATE "/tmp/par-shell-out-XXXXXX"

void showPrompt();
int main(int argc, char** argv) {
  int outfileid;
  char * c;
  char buffer[BUFFER_LEN];
  char * buffer_aux;
  char mktemp_filename[PATH_MAX];
  char mktemp_dir[PATH_MAX];
  int len, pid, mode;
  char * orig;
  int inputPipe;

  int totalProc;
  double totalTime;
  pid = getpid();
  if (argc!=2) {
    fprintf(stderr, "Modo de utilizacao: %s <pathname para o pipe>", argv[0]);
    exit(-1);
  }
  if ((outfileid = open(argv[1], O_WRONLY)) < 0) {
    fprintf(stderr, "Could not create fifo %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  mode = 1;
  orig = (char*)&mode;
  memcpy(buffer, orig, sizeof(int));
  orig = (char*)&pid;
  memcpy(buffer+sizeof(int), orig, sizeof(int));
  TESTTRUE(write(outfileid, buffer, 2*sizeof(int))==2*sizeof(int), "Erro na escrite para o pipe (" _AT_ ")\n");
  showPrompt();
  buffer_aux = buffer + 3*sizeof(int);
  while ((c = fgets(buffer_aux, BUFFER_LEN - 3*sizeof(int), stdin)) > 0) {
    //printf("\"%s\"\n", buffer_aux);
    if (c == NULL || *c == EOF) {
      break;
    }
    if (strcmp("exit\n", buffer_aux)==0) {
      break;
    }
    if (strcmp("exit-global\n", buffer_aux)==0) {
      mode = 3;
      orig = (char*)&mode;
      memcpy(buffer, orig, sizeof(int));
      TESTTRUE(write(outfileid, buffer, sizeof(int))==sizeof(int), "Erro na escrite para o pipe (" _AT_ ")\n");
      pause(); //o processo vai morrer, mas ate la pausamos
      exit(-1);
    }
    if (strcmp("stats\n", buffer_aux)==0) {
      strcpy(mktemp_dir, MKTEMP_TEMPLATE);
      TESTTRUE(mkdtemp(mktemp_dir)!=NULL, "Erro na criação do diretorio temporário (" _AT_ "\n");
      strncpy(mktemp_filename, mktemp_dir, PATH_MAX);
      strncpy(mktemp_filename+strlen(mktemp_filename), "/out", PATH_MAX-strlen(mktemp_filename));
      if (mkfifo(mktemp_filename, 0660) <0) {
        fprintf(stderr, "Could not create fifo " INPUT_FILE "\n");
        exit(EXIT_FAILURE);
      }

      len = strlen(mktemp_filename);
      strncpy(buffer_aux, mktemp_filename, BUFFER_LEN - 3*sizeof(int));
      mode = 4;
      orig = (char*)&mode;
      memcpy(buffer, orig, sizeof(int));
      orig = (char*)&len;
      memcpy(buffer+sizeof(int), orig, sizeof(int));
      orig = (char*)&pid;
      memcpy(buffer+2*sizeof(int), orig, sizeof(int));
      TESTTRUE(write(outfileid, buffer, 3*sizeof(int) + len)==3*sizeof(int) + len, "Erro na escrita para o pipe (" _AT_ ")\n");

      if ((inputPipe = open(mktemp_filename, O_RDONLY)) < 0) {
        fprintf(stderr, "Could not create fifo " INPUT_FILE "\n");
        exit(EXIT_FAILURE);
      }
      TESTTRUE((read(inputPipe, &totalProc, sizeof(int))==sizeof(int)), "Erro no formato do pipe (" _AT_ ")\n");
      TESTTRUE((read(inputPipe, &totalTime, sizeof(double))==sizeof(double)), "Erro no formato do pipe (" _AT_ ")\n");

      printf("Numero total de processos: %d\n"
             "Tempo total: %f\n", totalProc, totalTime);

      close(inputPipe);//nao faz sentido testarmos isto, pois e um ficheiro temporario
      unlink(mktemp_filename);//nao faz sentido testarmos isto, pois e um ficheiro temporario
      unlink(mktemp_dir);//nao faz sentido testarmos isto, pois e um ficheiro temporario
      showPrompt();
      continue;
    }

    //Caso comando normal:
    len = strlen(buffer_aux);
    len++;
    mode = 0;
    orig = (char*)&mode;
    memcpy(buffer, orig, sizeof(int));
    orig = (char*)&len;
    memcpy(buffer+sizeof(int), orig, sizeof(int));
    orig = (char*)&pid;
    memcpy(buffer+2*sizeof(int), orig, sizeof(int));
    TESTTRUE(write(outfileid, buffer, 3*sizeof(int) + len)==3*sizeof(int) + len, "Erro na escrite para o pipe (" _AT_ ")\n");
    showPrompt();
  }
  mode = 2;
  orig = (char*)&mode;
  memcpy(buffer, orig, sizeof(int));
  orig = (char*)&pid;
  memcpy(buffer+sizeof(int), orig, sizeof(int));
  TESTTRUE(write(outfileid, buffer, 2*sizeof(int))==2*sizeof(int), "Erro na escrite para o pipe (" _AT_ ")\n");
  return close(outfileid);
}

/*
  Outputs to stdout the current working directory
*/
void showPrompt() {
  char *pwd = getcwd(NULL,420);
  if (pwd != NULL) {
    printf("%s$ ", pwd);
    free(pwd);
  } else {
    printf("unk$ ");
  }
}
