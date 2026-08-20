/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_common.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	simulation_stopped(t_sim *sim)
{
	int	simstate;

	pthread_mutex_lock(&sim->state_mutex);
	simstate = sim->simstate;
	pthread_mutex_unlock(&sim->state_mutex);
	return (simstate);
}

void	update_dongle_state(long long now, t_dongle *dongle)
{
	if (dongle->state == DONGLE_STATE_COOLDOWN
		&& dongle->cooldown_end <= now)
		dongle->state = DONGLE_STATE_AVAILABLE;
}

int	try_to_push(t_dongle *dongle, t_coder *coder, long long data)
{
	dongle->pq->cmp = coder->cmp;
	if (push_pq(dongle->pq, coder->coder_id, data) == SUCCESS)
		return (SUCCESS);
	pthread_mutex_lock(&coder->sim->state_mutex);
	if (coder->sim->simstate == IN_PROGRESS)
		coder->sim->simstate = INTERNAL_ERROR;
	pthread_mutex_unlock(&coder->sim->state_mutex);
	return (FAILURE);
}

long long	get_time_data(t_coder *coder)
{
	if (coder->is_edf)
		return (coder->last_compile_start
			+ coder->conf[TIME_TO_BURNOUT_MS] * 1000LL);
	return (0);
}

bool	is_dongle_for_me(t_coder *coder, t_dongle *dongle)
{
	if (dongle->pq->size == 0)
		return (false);
	if (dongle->pq->queue[0]->coder_id != coder->coder_id)
		return (false);
	return (dongle->state == DONGLE_STATE_AVAILABLE);
}
