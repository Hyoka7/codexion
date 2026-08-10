/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mainloop.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	create_workers(t_sim *sim, pthread_t *monitor,
		bool *monitor_created)
{
	int	created;

	created = 0;
	while (created < sim->conf[NUM_OF_CODERS])
	{
		if (pthread_create(&sim->coders[created].thread_id, NULL,
				coder_routine, &sim->coders[created]) != 0)
			break ;
		created++;
	}
	sim->timer_created = (created == sim->conf[NUM_OF_CODERS]
			&& sim->conf[NUM_OF_CODERS] > 1
			&& pthread_create(&sim->timer_thread, NULL,
				scheduler_timer_routine, sim) == 0);
	*monitor_created = ((sim->timer_created
				|| sim->conf[NUM_OF_CODERS] == 1)
			&& pthread_create(monitor, NULL, monitor_routine, sim) == 0);
	if (!*monitor_created)
	{
		pthread_mutex_lock(&sim->state_mutex);
		sim->simstate = INTERNAL_ERROR;
		pthread_mutex_unlock(&sim->state_mutex);
		stop_and_wake(sim);
	}
	return (created);
}

static bool	join_workers(t_sim *sim, pthread_t *monitor, int created,
		bool monitor_created)
{
	bool	join_success;

	join_success = true;
	if (monitor_created && pthread_join(*monitor, NULL) != 0)
		join_success = false;
	while (created-- > 0)
	{
		if (pthread_join(sim->coders[created].thread_id, NULL) != 0)
			join_success = false;
	}
	if (sim->timer_created
		&& pthread_join(sim->timer_thread, NULL) != 0)
		join_success = false;
	return (join_success);
}

static int	finish_simulation(t_sim *sim)
{
	int	result;

	pthread_mutex_lock(&sim->state_mutex);
	if (sim->simstate == INTERNAL_ERROR)
		result = FAILURE;
	else
		result = SUCCESS;
	pthread_mutex_unlock(&sim->state_mutex);
	free_priority_queue(sim->requests);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_cond_destroy(&sim->state_cond);
	pthread_mutex_destroy(&sim->state_mutex);
	return (result);
}

int	mainloop(int *conf, t_dongle *dongles, t_coder *coders, char *scheduler)
{
	t_sim		sim;
	pthread_t	monitor;
	int			created;
	bool		monitor_created;

	if (init_sim_data(&sim, conf, dongles, coders) == FAILURE)
		return (FAILURE);
	init_all_coders(&sim, scheduler);
	created = create_workers(&sim, &monitor, &monitor_created);
	if (!join_workers(&sim, &monitor, created, monitor_created))
		return (INTERNAL_ERROR);
	return (finish_simulation(&sim));
}
