/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	sim_sleep(t_sim *sim, long long duration)
{
	long long	end;

	end = get_current_ms() + duration;
	while (get_current_ms() < end)
	{
		if (simulation_stopped(sim))
			return (FAILURE);
		usleep(500);
	}
	return (SUCCESS);
}

static int	start_compile(t_coder *coder)
{
	long long	current_time;

	pthread_mutex_lock(&coder->sim->state_mutex);
	current_time = get_current_ms();
	if (coder->sim->simstate != IN_PROGRESS
		|| current_time > coder->last_compile_start
		+ coder->conf[TIME_TO_BURNOUT_MS])
	{
		pthread_mutex_unlock(&coder->sim->state_mutex);
		release_dongles(coder);
		return (FAILURE);
	}
	coder->last_compile_start = current_time;
	pthread_cond_broadcast(&coder->sim->state_cond);
	pthread_mutex_unlock(&coder->sim->state_mutex);
	print_status(coder, "is compiling");
	return (SUCCESS);
}

static int	finish_compile(t_coder *coder)
{
	if (sim_sleep(coder->sim, coder->conf[TIME_TO_COMPILE_MS]) == FAILURE)
	{
		release_dongles(coder);
		return (FAILURE);
	}
	pthread_mutex_lock(&coder->sim->state_mutex);
	coder->compile_count++;
	if (coder->compile_count == coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
		coder->done = true;
	pthread_cond_broadcast(&coder->sim->state_cond);
	pthread_mutex_unlock(&coder->sim->state_mutex);
	release_dongles(coder);
	return (SUCCESS);
}

static int	debug_and_refactor(t_coder *coder)
{
	print_status(coder, "is debugging");
	if (sim_sleep(coder->sim, coder->conf[TIME_TO_DEBUG_MS]) == FAILURE)
		return (FAILURE);
	print_status(coder, "is refactoring");
	if (sim_sleep(coder->sim, coder->conf[TIME_TO_REFACTOR_MS]) == FAILURE)
		return (FAILURE);
	return (SUCCESS);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (wait_for_simulation_start(coder->sim) == FAILURE)
		return (NULL);
	while (coder->compile_count < coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
	{
		if (get_dongles(coder) == FAILURE || start_compile(coder) == FAILURE)
			return (NULL);
		if (finish_compile(coder) == FAILURE)
			return (NULL);
		if (coder->done)
			break ;
		if (debug_and_refactor(coder) == FAILURE)
			return (NULL);
	}
	return (NULL);
}
