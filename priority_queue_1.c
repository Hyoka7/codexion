#include "codexion.h"

t_priority_queue *create_priority_queue()
{
    t_priority_queue *pq;

    pq = malloc(sizeof(t_priority_queue));
    if (!pq)
        return (NULL);
    pq->root = NULL;
    pq->size = 0;
    return (pq);
}

static t_pqnode *create_pq_node(int id, float priority)
{
    t_pqnode *new_node;

    new_node = malloc(sizeof(t_pqnode));
    if (!new_node)
        return (NULL);
    new_node->id = id;
    new_node->priority = priority;
    new_node->left = NULL;
    new_node->right = NULL;
}

static void push_node(t_pqnode **head, t_pqnode *node)
{
    if (*head == NULL)
    {
        *head = node;
        return ;
    }
    if ((*head)->priority < node->priority)
    {
        push_node(&(*head)->right, node);
    }
    else
        push_node(&(*head)->left, node);
}

void push_priority_queue(t_priority_queue *queue, int id, float priority)
{
    t_pqnode *new_node;

    new_node = create_pq_node(id, priority);
    if (!new_node)
        return ;
    push_node(&queue->root, new_node);
    queue->size++;
}