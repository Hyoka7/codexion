/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	lock_coder_dongles(t_coder *coder, t_dongle **first,
		t_dongle **second)
{
	*first = coder->dongle_l;
	*second = coder->dongle_r;
	if ((*first)->dongle_id > (*second)->dongle_id)
	{
		*first = coder->dongle_r;
		*second = coder->dongle_l;
	}
	pthread_mutex_lock(&(*first)->mutex);
	pthread_mutex_lock(&(*second)->mutex);
}

void	unlock_coder_dongles(t_dongle *first, t_dongle *second)
{
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
}

bool	request_is_feasible(t_coder *coder, long long now)
{
	t_dongle	*first;
	t_dongle	*second;
	bool		available;

	if (now > coder->last_compile_start
		+ coder->conf[TIME_TO_BURNOUT_MS] * 1000LL)
		return (false);
	lock_coder_dongles(coder, &first, &second);
	update_dongle_state(now, first);
	update_dongle_state(now, second);
	available = (first->state == DONGLE_STATE_AVAILABLE
			&& second->state == DONGLE_STATE_AVAILABLE);
	unlock_coder_dongles(first, second);
	return (available);
}

long long	next_scheduler_wake(t_sim *sim)
{
	long long	wake_time;
	long long	now;
	int			index;

	wake_time = 0;
	now = get_current_us();
	index = 0;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		pthread_mutex_lock(&sim->dongles[index].mutex);
		update_dongle_state(now, &sim->dongles[index]);
		if (sim->dongles[index].state == DONGLE_STATE_COOLDOWN
			&& (wake_time == 0
				|| sim->dongles[index].cooldown_end < wake_time))
			wake_time = sim->dongles[index].cooldown_end;
		pthread_mutex_unlock(&sim->dongles[index].mutex);
		index++;
	}
	return (wake_time);
}
