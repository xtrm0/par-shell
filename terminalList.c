/*
 * list.c - implementation of the integer list functions
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "terminalList.h"



TerminalList* lst_new()
{
   TerminalList *list;
   list = (TerminalList*) malloc(sizeof(TerminalList));
   list->first = NULL;
   return list;
}


void lst_destroy(TerminalList *list)
{
	struct lst_iitem *item, *nextitem;
	item = list->first;
	while (item != NULL){
		nextitem = item->next;
    kill(item->pid, SIGINT);
		free(item);
		item = nextitem;
	}
	free(list);
}


void lst_insert(TerminalList *list, int pid)
{
	lst_iitem_t *item;
	item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
	item->pid = pid;
	item->next = list->first;
	list->first = item;
}


void lst_remove(TerminalList *list, int pid)
{
  struct lst_iitem *item, *nextItem;
  if (list->first == NULL) return;
  if (list->first->pid == pid) {
    item = list->first;
    list->first = list->first->next;
    free(item);
    return;
  }
	item = list->first;
	while (item->next != NULL) {
		if(item->next->pid == pid) {
      nextItem = item->next;
      item->next = item->next->next;
      free(nextItem);
      return;
    }
    item = item->next;
	}
}
