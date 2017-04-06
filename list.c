#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "list.h"
#include "search.h"

struct list *list_alloc() {
  struct list *list;

  list = malloc(sizeof(struct list));
  return list;
}

int list_push(struct list *list, void *elt) {

  if (list->nelts == list->cap) {
    list->cap *= 2;
    list->elts = realloc(list->elts, list->size * list->cap);
    if (!list->elts)
      return -1;
  }

  memmove((char *)list->elts + (list->size * list->nelts), elt, list->size);
  printf("list->elts[i].filepath %s\n", ((struct match *)list->elts)[list->nelts].filepath);
  list->nelts++;

  return 0;
}

int list_init(struct list *list, size_t cap, size_t size) {

  list->elts = malloc(cap * size);
  if (!list->elts) {
    return -1;
  }
  list->size = size;
  list->cap = cap;
  list->nelts = 0;
  return 0;
}
