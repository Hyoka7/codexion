/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue_1.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 21:06:36 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_pq	*create_pq(int num_coders)
{
	t_pq	*queue;

	queue = malloc(sizeof(t_pq));
	if (!queue)
		return (NULL);
	queue->queue = malloc(sizeof(t_pqnode *) * num_coders);
	if (!queue->queue)
	{
		free(queue);
		return (NULL);
	}
	queue->size = 0;
	queue->next_fifo_rank = 0;
	queue->cmp = NULL;
	return (queue);
}

static t_pqnode	*create_node(t_pq *queue, int coder_id, long long data)
{
	t_pqnode	*node;

	node = malloc(sizeof(t_pqnode));
	if (!node)
		return (NULL);
	if (queue->size == 0)
		queue->next_fifo_rank = 0;
	node->coder_id = coder_id;
	node->fifo_rank = queue->next_fifo_rank++;
	node->queue_seconds = data;
	return (node);
}

static void	move_node_up(t_pq *queue, size_t node_index)
{
	size_t		parent_index;
	t_pqnode	*temporary;

	while (node_index > 0)
	{
		parent_index = (node_index - 1) / 2;
		if (queue->cmp(queue->queue[node_index],
				queue->queue[parent_index]) >= 0)
			break ;
		temporary = queue->queue[parent_index];
		queue->queue[parent_index] = queue->queue[node_index];
		queue->queue[node_index] = temporary;
		node_index = parent_index;
	}
}

int	push_pq(t_pq *queue, int coder_id, long long data)
{
	t_pqnode	*node;

	node = create_node(queue, coder_id, data);
	if (!node)
		return (FAILURE);
	queue->queue[queue->size] = node;
	queue->size++;
	move_node_up(queue, queue->size - 1);
	return (SUCCESS);
}
