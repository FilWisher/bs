#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <regex.h>

#include "list.h"
#include "search.h"

void usage() {
  fprintf(stderr, "bs <query> <filepath>\n");
}

int main(int argc, char **argv) {

  struct search search;
  struct match *matches;
  size_t i;
  
  if (argc < 3) {
    usage();
    return -1;
  }

  printf("bs %s %s\n", argv[1], argv[2]);
  if (search_init(&search, argv[1], NULL) == -1) {
    fprintf(stderr, "couldn't init search\n");
    return -1;
  }

  if (search_file(&search, argv[2]) == -1) {
    fprintf(stderr, "search failed\n");
    return -1;
  }

  matches = (struct match *)search.matches.elts;
  for (i = 0; i < search.matches.nelts; i++) {
    match_print(matches[i]);
  }

  return 0;
}
