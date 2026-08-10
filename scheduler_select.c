/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_select.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	request_heads_both_queues(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	bool		is_head;

	lock_coder_dongles(coder, &first, &second);
	is_head = (first->pq->size > 0 && second->pq->size > 0
			&& first->pq->queue[0]->coder_id == coder->coder_id
			&& second->pq->queue[0]->coder_id == coder->coder_id);
	unlock_coder_dongles(first, second);
	return (is_head);
}

static void	grant_request(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	lock_coder_dongles(coder, &first, &second);
	free(pop_pq(first->pq));
	free(pop_pq(second->pq));
	first->state = DONGLE_STATE_USING;
	second->state = DONGLE_STATE_USING;
	coder->has_dongles = true;
	coder->request_pending = false;
	unlock_coder_dongles(first, second);
}

void	schedule_requests(t_sim *sim)
{
	int	index;
	int	granted;

	granted = false;
	index = 0;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		if (sim->coders[index].request_pending
			&& request_heads_both_queues(&sim->coders[index])
			&& request_is_feasible(&sim->coders[index], get_current_ms()))
		{
			grant_request(&sim->coders[index]);
			granted = true;
			index = 0;
		}
		else
			index++;
	}
	if (granted)
		pthread_cond_broadcast(&sim->state_cond);
}
