/*
 * list.h - definitions and declarations of the integer list
 */

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>

typedef struct lst_iitem {
   int pid;
   struct lst_iitem *next;
} lst_iitem_t;

typedef struct {
   lst_iitem_t * first;
} TerminalList;

TerminalList* lst_new();

void lst_destroy(TerminalList *);

void lst_insert(TerminalList *list, int pid);

void lst_remove(TerminalList *list, int pid);

#endif
