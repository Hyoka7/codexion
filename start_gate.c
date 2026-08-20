/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start_gate.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/19 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	wait_for_simulation_start(t_sim *sim)
{
	int	result;

	result = SUCCESS;
	pthread_mutex_lock(&sim->state_mutex);
	sim->ready_workers++;
	pthread_cond_broadcast(&sim->state_cond);
	while (!sim->start_ready && sim->simstate == IN_PROGRESS)
	{
		result = pthread_cond_wait(&sim->state_cond, &sim->state_mutex);
		if (result != SUCCESS)
		{
			sim->simstate = INTERNAL_ERROR;
			pthread_cond_broadcast(&sim->state_cond);
			break ;
		}
	}
	if (!sim->start_ready || sim->simstate != IN_PROGRESS)
		result = FAILURE;
	pthread_mutex_unlock(&sim->state_mutex);
	return (result);
}

static void	set_start_time(t_sim *sim)
{
	int	index;

	sim->start_time = get_current_us();
	index = 0;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		sim->coders[index].last_compile_start = sim->start_time;
		index++;
	}
}

int	start_simulation(t_sim *sim, int expected_workers)
{
	int	result;

	result = SUCCESS;
	pthread_mutex_lock(&sim->state_mutex);
	while (sim->ready_workers < expected_workers
		&& sim->simstate == IN_PROGRESS)
	{
		result = pthread_cond_wait(&sim->state_cond, &sim->state_mutex);
		if (result != SUCCESS)
			sim->simstate = INTERNAL_ERROR;
	}
	if (sim->simstate == IN_PROGRESS)
	{
		set_start_time(sim);
		sim->start_ready = true;
	}
	else
		result = FAILURE;
	pthread_cond_broadcast(&sim->state_cond);
	pthread_mutex_unlock(&sim->state_mutex);
	return (result);
}
