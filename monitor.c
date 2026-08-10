/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	stop_and_wake(t_sim *sim)
{
	int	index;

	pthread_mutex_lock(&sim->state_mutex);
	pthread_cond_broadcast(&sim->state_cond);
	pthread_mutex_unlock(&sim->state_mutex);
	index = 0;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		pthread_mutex_lock(&sim->dongles[index].mutex);
		pthread_cond_broadcast(&sim->dongles[index].cond);
		pthread_mutex_unlock(&sim->dongles[index].mutex);
		index++;
	}
}

static int	find_burnout(t_sim *sim, long long now, int *all_done)
{
	int	index;

	index = 0;
	*all_done = true;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		if (!sim->coders[index].done)
			*all_done = false;
		if (!sim->coders[index].done
			&& now >= sim->coders[index].last_compile_start
			+ sim->conf[TIME_TO_BURNOUT_MS])
			return (index);
		index++;
	}
	return (-1);
}

static bool	monitor_should_stop(t_sim *sim, int *burned_index)
{
	int	all_done;

	*burned_index = find_burnout(sim, get_current_ms(), &all_done);
	if (*burned_index < 0 && !all_done)
		return (false);
	if (*burned_index >= 0)
		sim->simstate = BURN_OUT;
	else
		sim->simstate = COMPLETED;
	return (true);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		burned_index;

	sim = (t_sim *)arg;
	while (1)
	{
		pthread_mutex_lock(&sim->state_mutex);
		if (sim->simstate != IN_PROGRESS
			|| monitor_should_stop(sim, &burned_index))
		{
			pthread_mutex_unlock(&sim->state_mutex);
			if (sim->simstate == BURN_OUT)
				print_burnout(sim, burned_index);
			stop_and_wake(sim);
			return (NULL);
		}
		pthread_mutex_unlock(&sim->state_mutex);
		usleep(500);
	}
}
