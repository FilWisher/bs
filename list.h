// list iteration: 
// 
// struct list *list = ...
// data = list->elts
// for (i = 0; i < list->nelts; i++) {
//   ... data[i] ...
// }

struct list {
  void *elts;
  size_t nelts;
  size_t cap;
  // size of list elt
  size_t size;
};

int list_init(struct list *list, size_t cap, size_t size);
struct list *list_alloc();
int list_push(struct list *list, void *elt);
