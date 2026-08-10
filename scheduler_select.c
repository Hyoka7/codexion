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

static int	find_best_feasible(t_sim *sim)
{
	t_pqnode	*node;
	int			best;
	size_t		index;
	long long	now;

	best = -1;
	index = 0;
	now = get_current_ms();
	while (index < sim->requests->size)
	{
		node = sim->requests->queue[index];
		if (request_is_feasible(&sim->coders[node->coder_id - 1], now)
			&& (best < 0 || sim->requests->cmp(node,
					sim->requests->queue[best]) < 0))
			best = index;
		index++;
	}
	return (best);
}

static void	grant_request(t_sim *sim, int request_index)
{
	t_pqnode	*request;
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	request = remove_pq_at(sim->requests, request_index);
	coder = &sim->coders[request->coder_id - 1];
	lock_coder_dongles(coder, &first, &second);
	first->state = DONGLE_STATE_USING;
	second->state = DONGLE_STATE_USING;
	coder->has_dongles = true;
	unlock_coder_dongles(first, second);
	free(request);
}

void	schedule_requests(t_sim *sim)
{
	int	best;
	int	granted;

	granted = false;
	best = find_best_feasible(sim);
	while (best >= 0)
	{
		grant_request(sim, best);
		granted = true;
		best = find_best_feasible(sim);
	}
	if (granted)
		pthread_cond_broadcast(&sim->state_cond);
}
