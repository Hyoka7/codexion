/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_single.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	fail_single_wait(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&coder->sim->state_mutex);
	coder->sim->simstate = INTERNAL_ERROR;
	pthread_mutex_unlock(&coder->sim->state_mutex);
	pthread_mutex_unlock(&dongle->mutex);
	return (FAILURE);
}

static int	get_single_dongle(t_dongle *dongle, t_coder *coder)
{
	int	wait_result;

	pthread_mutex_lock(&dongle->mutex);
	if (try_to_push(dongle, coder, get_time_data(coder)) == FAILURE)
		return (pthread_mutex_unlock(&dongle->mutex), FAILURE);
	while (!simulation_stopped(coder->sim))
	{
		update_dongle_state(get_current_us(), dongle);
		if (is_dongle_for_me(coder, dongle))
		{
			dongle->state = DONGLE_STATE_USING;
			print_status(coder, "has taken a dongle");
			free(pop_pq(dongle->pq));
			return (pthread_mutex_unlock(&dongle->mutex), SUCCESS);
		}
		wait_result = pthread_cond_wait(&dongle->cond, &dongle->mutex);
		if (wait_result != SUCCESS)
			return (fail_single_wait(coder, dongle));
	}
	return (pthread_mutex_unlock(&dongle->mutex), FAILURE);
}

int	sequence_get_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (get_single_dongle(first, coder) == FAILURE)
		return (FAILURE);
	if (get_single_dongle(second, coder) == FAILURE)
	{
		release_dongles(coder);
		return (FAILURE);
	}
	return (SUCCESS);
}
