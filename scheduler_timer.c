/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_timer.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	sleep_until_wake(t_sim *sim, long long wake_time)
{
	long long	now;
	long long	delay;

	now = get_current_us();
	while (now < wake_time && !simulation_stopped(sim))
	{
		delay = wake_time - now;
		if (delay > 11000)
			usleep(10000);
		else if (delay > 1000)
			usleep(delay - 500);
		else
			usleep(100);
		now = get_current_us();
	}
}

static long long	wait_for_cooldown(t_sim *sim)
{
	long long	wake_time;

	wake_time = next_scheduler_wake(sim);
	while (sim->simstate == IN_PROGRESS && wake_time == 0)
	{
		if (pthread_cond_wait(&sim->state_cond, &sim->state_mutex) != SUCCESS)
		{
			sim->simstate = INTERNAL_ERROR;
			return (0);
		}
		wake_time = next_scheduler_wake(sim);
	}
	return (wake_time);
}

void	*scheduler_timer_routine(void *arg)
{
	t_sim		*sim;
	long long	wake_time;

	sim = (t_sim *)arg;
	if (wait_for_simulation_start(sim) == FAILURE)
		return (NULL);
	while (1)
	{
		pthread_mutex_lock(&sim->state_mutex);
		wake_time = wait_for_cooldown(sim);
		if (sim->simstate != IN_PROGRESS)
			return (pthread_mutex_unlock(&sim->state_mutex), NULL);
		pthread_mutex_unlock(&sim->state_mutex);
		sleep_until_wake(sim, wake_time);
		pthread_mutex_lock(&sim->state_mutex);
		if (sim->simstate == IN_PROGRESS)
			schedule_requests(sim);
		pthread_mutex_unlock(&sim->state_mutex);
	}
}
