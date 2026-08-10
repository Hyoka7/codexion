/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue_remove.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

static void	move_down_from(t_pq *queue, size_t parent)
{
	size_t		child;
	t_pqnode	*temporary;

	while (parent * 2 + 1 < queue->size)
	{
		child = smallest_child(queue, parent);
		if (queue->cmp(queue->queue[parent], queue->queue[child]) <= 0)
			break ;
		temporary = queue->queue[parent];
		queue->queue[parent] = queue->queue[child];
		queue->queue[child] = temporary;
		parent = child;
	}
}

static void	rebuild_heap(t_pq *queue)
{
	size_t	index;

	index = queue->size / 2;
	while (index > 0)
	{
		index--;
		move_down_from(queue, index);
	}
}

t_pqnode	*remove_pq_at(t_pq *queue, size_t index)
{
	t_pqnode	*result;

	if (!queue || index >= queue->size)
		return (NULL);
	result = queue->queue[index];
	queue->size--;
	if (index < queue->size)
	{
		queue->queue[index] = queue->queue[queue->size];
		rebuild_heap(queue);
	}
	return (result);
}
