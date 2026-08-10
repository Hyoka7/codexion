/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_sim_locks(t_sim *sim)
{
	if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
		return (FAILURE);
	if (pthread_cond_init(&sim->state_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->state_mutex);
		return (FAILURE);
	}
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
	{
		pthread_cond_destroy(&sim->state_cond);
		pthread_mutex_destroy(&sim->state_mutex);
		return (FAILURE);
	}
	return (SUCCESS);
}

static void	set_coder_data(t_sim *sim, char *scheduler, int index)
{
	t_coder	*coder;

	coder = &sim->coders[index];
	coder->coder_id = index + 1;
	coder->request_time = sim->start_time;
	coder->last_compile_start = sim->start_time;
	coder->conf = sim->conf;
	coder->is_edf = (strcmp(scheduler, "edf") == 0);
	if (coder->is_edf)
		coder->cmp = cmp_edf;
	else
		coder->cmp = cmp_fifo;
	coder->sim = sim;
	coder->dongle_l = &sim->dongles[index];
	coder->dongle_r = &sim->dongles[(index + 1) % sim->conf[NUM_OF_CODERS]];
	if (sim->conf[NUMBERS_OF_COMPILES_REQUIRED] == 0)
		coder->done = true;
}

int	init_sim_data(t_sim *sim, int *conf, t_dongle *dongles,
		t_coder *coders)
{
	memset(sim, 0, sizeof(*sim));
	sim->conf = conf;
	sim->start_time = get_current_ms();
	sim->coders = coders;
	sim->dongles = dongles;
	if (init_sim_locks(sim) == FAILURE)
		return (FAILURE);
	sim->requests = create_pq(conf[NUM_OF_CODERS]);
	if (!sim->requests)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		pthread_cond_destroy(&sim->state_cond);
		pthread_mutex_destroy(&sim->state_mutex);
		return (FAILURE);
	}
	return (SUCCESS);
}

void	init_all_coders(t_sim *sim, char *scheduler)
{
	int	index;

	index = 0;
	while (index < sim->conf[NUM_OF_CODERS])
	{
		set_coder_data(sim, scheduler, index);
		index++;
	}
	if (strcmp(scheduler, "edf") == 0)
		sim->requests->cmp = cmp_edf;
	else
		sim->requests->cmp = cmp_fifo;
}
