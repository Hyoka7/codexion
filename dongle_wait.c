/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_wait.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <errno.h>

static long long	ft_max(long long first, long long second)
{
	if (first < second)
		return (second);
	return (first);
}

static long long	get_wake_time(t_dongle *first, t_dongle *second)
{
	long long	first_wake;
	long long	second_wake;

	first_wake = 0;
	second_wake = 0;
	if (first->state == DONGLE_STATE_COOLDOWN)
		first_wake = first->cooldown_end;
	if (second->state == DONGLE_STATE_COOLDOWN)
		second_wake = second->cooldown_end;
	return (ft_max(first_wake, second_wake));
}

static int	wait_on_state(t_sim *sim, long long wake_time)
{
	struct timespec	time_spec;
	int				result;

	if (wake_time == 0)
		result = pthread_cond_wait(&sim->state_cond, &sim->state_mutex);
	else
	{
		time_spec = convert_ms_to_timespec(wake_time);
		result = pthread_cond_timedwait(&sim->state_cond,
				&sim->state_mutex, &time_spec);
	}
	if (result != SUCCESS && result != ETIMEDOUT)
		sim->simstate = INTERNAL_ERROR;
	return (result);
}

int	wait_for_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	long long	wake_time;
	int			result;

	wake_time = get_wake_time(first, second);
	pthread_mutex_lock(&coder->sim->state_mutex);
	if (coder->sim->simstate != IN_PROGRESS)
		return (pthread_mutex_unlock(&coder->sim->state_mutex), FAILURE);
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	result = wait_on_state(coder->sim, wake_time);
	pthread_mutex_unlock(&coder->sim->state_mutex);
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	if (result != SUCCESS && result != ETIMEDOUT)
		return (FAILURE);
	return (SUCCESS);
}
