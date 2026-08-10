/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>

void	print_status(t_coder *coder, const char *status)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	if (sim->simstate == IN_PROGRESS)
		printf("%lld %d %s\n", get_elapsed_ms(sim->start_time),
			coder->coder_id, status);
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	print_dongle_pair(t_coder *coder)
{
	t_sim		*sim;
	long long	elapsed;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	if (sim->simstate == IN_PROGRESS)
	{
		elapsed = get_elapsed_ms(sim->start_time);
		printf("%lld %d has taken a dongle\n", elapsed, coder->coder_id);
		printf("%lld %d has taken a dongle\n", elapsed, coder->coder_id);
	}
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	print_burnout(t_sim *sim, int index)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%lld %d burned out\n", get_elapsed_ms(sim->start_time),
		sim->coders[index].coder_id);
	pthread_mutex_unlock(&sim->log_mutex);
}
