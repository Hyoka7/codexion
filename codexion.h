#include <stdlib.h>

typedef struct s_pqnode
{
  int id;
  float priority;
  struct s_pqnode *left;
  struct s_pqnode *right;
} t_pqnode;

typedef struct s_priority_queue
{
  t_pqnode *root;
  int size;
} t_priority_queue;

t_priority_queue *create_priority_queue();
void push_priority_queue(t_priority_queue *queue, int id, float priority);
void pop_priority_queue(t_priority_queue *queue, int *res_id, float *res_priority);
void free_priority_queue(t_priority_queue *queue);
