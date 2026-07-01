/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue_2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 21:06:43 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/01 20:34:21 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_priority_queue(t_pq *pq)
{
	size_t	pq_size;
	size_t	index;

	if (!pq)
	{
		return ;
	}
	pq_size = pq->size;
	index = 0;
	while (index < pq_size)
	{
		free(pq->queue[index++]);
	}
	free(pq->queue);
	free(pq);
}
