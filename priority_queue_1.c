/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue_1.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 21:06:36 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/05 20:45:51 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_pq *create_pq(int num_coders)
{
	t_pq *pq;

	pq = malloc(sizeof(t_pq));
	if (!pq)
	{
		return (NULL);
	}
	pq->queue = malloc(sizeof(t_pqnode *) * num_coders);
	if (!pq->queue)
	{
		free(pq);
		return (NULL);
	}
	pq->size = 0;
	pq->next_fifo_rank = 0;
	pq->cmp = NULL;
	return (pq);
}

static t_pqnode *create_new_node(t_pq *pq, int coder_id, long long time_data)
{
	t_pqnode *new_node;

	new_node = malloc(sizeof(t_pqnode));
	if (!new_node)
	{
		return (NULL);
	}
	if (pq->size == 0)
		pq->next_fifo_rank = 0;
	new_node->coder_id = coder_id;
	new_node->fifo_rank = pq->next_fifo_rank;
	pq->next_fifo_rank += 1;
	new_node->queue_seconds = time_data;
	return (new_node);
}

int push_pq(t_pq *queue, int coder_id, long long time_data)
{
	size_t new_node_i;
	size_t cmp_i;
	t_pqnode *temp;
	t_pqnode *new_node;

	new_node = create_new_node(queue, coder_id, time_data);
	if (!new_node)
		return (FAILURE);
	queue->queue[queue->size++] = new_node;
	new_node_i = queue->size - 1;
	while (new_node_i)
	{
		cmp_i = (new_node_i - 1) / 2;
		if (queue->cmp(queue->queue[new_node_i], queue->queue[cmp_i]) < 0)
		{
			temp = queue->queue[cmp_i];
			queue->queue[cmp_i] = queue->queue[new_node_i];
			queue->queue[new_node_i] = temp;
			new_node_i = cmp_i;
		}
		else
			break;
	}
	return (SUCCESS);
}

static void swap_pqnode(t_pqnode **a, t_pqnode **b)
{
	t_pqnode *temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

t_pqnode *pop_pq(t_pq *queue)
{
	t_pqnode *res;
	size_t left;
	size_t right;
	size_t parent;
	size_t child;

	if (!queue || !queue->size)
	{
		return (NULL);
	}
	res = queue->queue[0];
	queue->queue[0] = queue->queue[--queue->size];
	parent = 0;
	while (parent < queue->size)
	{
		left = 2 * parent + 1;
		if (left >= queue->size)
			break;
		right = 2 * parent + 2;
		if (right >= queue->size || queue->cmp(queue->queue[left],
											   queue->queue[right]) <= 0)
			child = left;
		else
			child = right;
		if (queue->cmp(queue->queue[parent], queue->queue[child]) > 0)
		{
			swap_pqnode(&(queue->queue[parent]), &(queue->queue[child]));
			parent = child;
		}
		else
			break;
	}
	return (res);
}
