#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>

#include "list.h"
#include "search.h"

struct search *
search_alloc()
{
  return malloc(sizeof(struct search));
}

mode_t
file_mode(const char *filepath)
{
  struct stat sp;

  if (stat(filepath, &sp) == -1)
    return -1;

  return sp.st_mode;
}

void
search_print(const struct search *search)
{
  printf("%s\t%zu matches\t%zu children\n",
	 search->query,
	 search->matches.nelts,
	 search->children.nelts);
}

int
search_init(struct search *search, char *query, struct search *parent)
{

  search->parent = parent;

  search->query = strdup(query);
  if (!search->query)
    return -1;

  if (regcomp(&search->re, query, REG_EXTENDED) != 0) {
    free(search->query);
    return -1;
  }

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

int
query_match(struct search *search, char *line, int len, int pos[2])
{

  regmatch_t pmatch;

  if (regexec(&search->re, line, 1, &pmatch, 0) != 0) {
    return 0;
  }

  pos[0] = pmatch.rm_so;
  pos[1] = pmatch.rm_eo - pos[0];
  return 1;
}

struct match *
match_create(char *filepath, uint64_t lineno, char *line, uint64_t off, uint64_t len)
{
  struct match *match = malloc(sizeof(struct match));
  if (!match)
    return NULL;

  match->filepath = strdup(filepath);
  if (!match->filepath) {
    free(match);
    return NULL;
  }

  // FIXME: this doesn't have to be so inefficient
  match->text = strndup(line, strlen(line)-1);
  if (!match->text) {
    free(match->filepath);
    free(match);
    return NULL;
  }

  match->line = lineno;
  match->off = off;
  match->len = len;

  return match;
}

int
search_file(struct search *search, char *filepath)
{
  FILE *file;
  char *line = NULL;
  size_t linesize = 0;
  ssize_t len;
  int pos[2];
  int linecount = 1;
  struct match *m;
  mode_t mode;

  if ((mode = file_mode(filepath)) == -1) {
    return -1;
  }

  if (mode & S_IFDIR) {
    return search_children(search, filepath);
  } else if (mode & S_IFREG) {

    file = fopen(filepath, "r");
    if (!file)
	return -1;

    while ((len = getline(&line, &linesize, file)) != -1) {
	if (query_match(search, line, len, pos)) {
	    m = match_create(filepath, linecount, line, pos[0], pos[1]);
	    if (!m) {
		free(line);
		return -1;
	    }
	    if (list_push(&search->matches, m) == -1) {
		free(line);
		return -1;
	    }
	}
	linecount++;
    }

    free(line);
  }
  return 0;
}

int
search_children(struct search *search, const char *filepath)
{
  DIR *dir;
  struct dirent *dent;
  char path[FILENAME_MAX];
  size_t len = strlen(filepath);
  struct stat sp;

  if (len >= FILENAME_MAX)
    return -1;

  int (*search_fn)(struct search *search, char *filepath);

  strncpy(path, filepath, len);

  // append / if not already there
  if (path[len-1] != '/')
    path[len++] = '/';

  if ((dir = opendir(filepath)) == NULL)
    return -1;

  while ((dent = readdir(dir))) {

    // skip dotfiles
    if (dent->d_name[0] == '.')
      continue;

    strncpy(path+len, dent->d_name, FILENAME_MAX - len);
    printf("searching %s\n", path);

    if ((stat(path, &sp)) == -1)
      return -1;

    if (sp.st_mode & S_IFDIR)
      search_fn = search_children;
    else if (sp.st_mode & S_IFREG)
      search_fn = search_file;
    else
      continue;

    if (search_fn(search, path) == -1)
      return -1;
  }

  return 0;
}

void
match_print(struct match match)
{
  printf("%s:%d: %s\n",
	 match.filepath,
	 (int)match.line,
	 match.text);
}

