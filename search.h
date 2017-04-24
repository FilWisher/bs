struct search {
  struct search *parent;
  char *query;

  regex_t re;

  struct list matches;
  struct list children;
};

struct match {
  char *filepath;

  char *text;

  uint64_t line;
  uint64_t off;
  uint64_t len;
};

struct search *search_alloc();
int search_init(struct search *search, char *query, struct search *parent);
int search_file(struct search *search, char *filepath);
void search_print(const struct search *search);
void match_print(struct match match);
int search_children(struct search *search, const char *filepath);
