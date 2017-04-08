#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <regex.h>

#include "list.h"
#include "search.h"

struct search_tree {
  struct search *root;
  struct search **current;
};

void usage()
{
  fprintf(stderr, "bs <query> <filepath>\n");
}

typedef int (*command)(struct search_tree *, int, char **);

struct named_command {
  char *name;
  command fn;
};

int
cmd_search(struct search_tree *st, int argc, char **argv)
{
  struct match *matches;
  struct search *search;
  size_t i;

  if (argc < 3) {
    usage();
    return 1;
  }

  search = search_alloc();
  if (!search)
    return -1;

  if (search_init(search, argv[1], *st->current) == -1) {
    fprintf(stderr, "couldn't init search\n");
    return 1;
  }

  if (search_file(search, argv[2]) == -1) {
    fprintf(stderr, "search failed\n");
    return 1;
  }

  matches = (struct match *)search->matches.elts;
  for (i = 0; i < search->matches.nelts; i++)
    match_print(matches[i]);

  *st->current = search;

  return 0;
}

int
cmd_list(struct search_tree *st, int argc, char **argv)
{
  struct search *search;

  for (search = *st->current; search != NULL; search = search->parent)
    search_print(search);

  return 0;
}

struct named_command cmds[] = {
  { "search", cmd_search },
  { "list", cmd_list }
};

command
get_command(char *name)
{
  size_t i;

  for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++)
    if (strcmp(name, cmds[i].name) == 0)
      return cmds[i].fn;

  return NULL;
}

void
free_argv(int argc, char **argv)
{
  int i;

  for (i = 0; i < argc; i++)
    free(argv[i]);
}

int
parse_line(char *line, int len, char *argv[10])
{
  int i;
  char *cp;
  int argc = 0;

  for (cp = line; cp < line+len && *cp != '\0'; cp++) {
    for (i = 0; !isspace(*(cp+i)) && cp+i < line+len; )
      i++;
    argv[argc++] = strndup(cp, i);
    cp += i;
    if (argc == 10)
      break;
  }

  return argc;
}

int
main()
{
  struct search_tree st;
  st.root = NULL;
  st.current = &st.root;

  char *line = NULL;
  size_t linesize = 0;
  ssize_t linelen;

  command cmd;

  int argc;
  char *argv[10];

  while ((linelen = getline(&line, &linesize, stdin)) != -1) {

    argc = parse_line(line, linelen, argv);
    if (argc < 1) {
      usage();
      break;
    }

    if ((cmd = get_command(argv[0])) == NULL) {
      printf("command not recognized: %s\n", argv[0]);
      continue;
    }

    // TODO: report what actually went wrong w/ error string
    if (cmd(&st, argc, argv) == -1)
      printf("error running command %s!\n", argv[0]);

  }

  free(line);
}
