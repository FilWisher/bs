#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "search.h"

struct search *search_alloc() {
  struct search *search = malloc(sizeof(struct search));
  return search;
}

int search_init(struct search *search, char *query, struct search *parent) {

  search->parent = parent;

  search->query = strdup(query);
  if (!search->query)
    return -1;

  if (list_init(&search->matches, 8, sizeof(struct match)) == -1) {
    free(search->query);
    return -1;
  }

  if (list_init(&search->children, 4, sizeof(struct search)) == -1) {
    free(search->query);
    return -1;
  }

  if (parent) {
    if (list_push(&parent->children, search) == -1) {
      // TODO: free line and free search
      return -1;
    } 
  }

  return 0;
}

int query_match(char *query, char *line, int len, int pos[2]) {
  pos[0] = 2; // off
  pos[1] = 5; // len
  return 1;
}

struct match *match_create(char *filepath, uint64_t line, uint64_t off, uint64_t len) {
  struct match *match = malloc(sizeof(struct match));
  if (!match)
    return NULL;
  // should this be dupped?
  match->filepath = strdup(filepath);
  if (!match->filepath)
    return NULL;

  match->line = line;
  match->off = off;
  match->len = len;

  return match;
}

int search_file(struct search *search, char *filepath) {
  FILE *file;
  char *line = NULL;
  size_t linesize = 0;
  ssize_t len;
  int pos[2];
  int linecount = 0;
  struct match *m;

  file = fopen(filepath, "r");
  if (!file)
    return -1;

  while ((len = getline(&line, &linesize, file)) != -1) {
    if (query_match(search->query, line, len, pos)) {
      m = match_create(filepath, linecount, pos[0], pos[1]);
      if (!m) {
	free(line);
	return -1;
      }
      if (list_push(&search->matches, m) == -1) {
	free(line);
	return -1;
      } else {
	printf("just pushed %s\n", ((struct match *)search->matches.elts)[search->matches.nelts-1].filepath);
      }
    }
    linecount++;
  }

  free(line);
  return 0;
}

void match_print(struct match match) {
  printf("%s %d:%d-%d\n",
	 match.filepath,
	 (int)match.line,
	 (int)match.off,
	 (int)match.len);
}
