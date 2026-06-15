#include "codexion.h"

void pop_priority_queue(t_priority_queue *queue, int *res_id, float *res_priority)
{
    t_pqnode *parent;
    t_pqnode *node;

    if (queue->root == NULL)
        return ;
    parent = NULL;
    node = queue->root;
    while (node->right)
    {
        parent = node;
        node = node->right;
    }
    *res_id = node->id;
    *res_priority = node->priority;
    if (parent == NULL)
        queue->root = node->left;
    else
        parent->right = node->left;
    free(node);
    queue->size--;
}

static void free_pq_node(t_pqnode *node)
{
    if (node)
    {
        free_pq_node(node->left);
        free_pq_node(node->right);
        free(node);
    }
    return ;
}

void free_priority_queue(t_priority_queue *queue)
{
    free_pq_node(queue->root);
    free(queue);
}
