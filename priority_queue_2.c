/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue_2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 21:06:43 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_nodes(t_pqnode **first, t_pqnode **second)
{
	t_pqnode	*temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

static size_t	smallest_child(t_pq *queue, size_t parent)
{
	size_t	left;
	size_t	right;

	left = parent * 2 + 1;
	right = parent * 2 + 2;
	if (right >= queue->size)
		return (left);
	if (queue->cmp(queue->queue[left], queue->queue[right]) <= 0)
		return (left);
	return (right);
}

static void	move_node_down(t_pq *queue)
{
	size_t	parent;
	size_t	child;

	parent = 0;
	while (parent * 2 + 1 < queue->size)
	{
		child = smallest_child(queue, parent);
		if (queue->cmp(queue->queue[parent], queue->queue[child]) <= 0)
			break ;
		swap_nodes(&queue->queue[parent], &queue->queue[child]);
		parent = child;
	}
}

t_pqnode	*pop_pq(t_pq *queue)
{
	t_pqnode	*result;

	if (!queue || queue->size == 0)
		return (NULL);
	result = queue->queue[0];
	queue->size--;
	if (queue->size > 0)
	{
		queue->queue[0] = queue->queue[queue->size];
		move_node_down(queue);
	}
	return (result);
}

void	free_priority_queue(t_pq *queue)
{
	size_t	index;

	if (!queue)
		return ;
	index = 0;
	while (index < queue->size)
	{
		free(queue->queue[index]);
		index++;
	}
	free(queue->queue);
	free(queue);
}
