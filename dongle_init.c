/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_init.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	free_initialized_dongles(t_dongle *dongles, int size)
{
	int	index;

	index = 0;
	while (index < size)
	{
		pthread_cond_destroy(&dongles[index].cond);
		pthread_mutex_destroy(&dongles[index].mutex);
		free_priority_queue(dongles[index].pq);
		index++;
	}
}

static t_dongle	*fail_dongle_init(t_dongle *dongles, int index, int stage)
{
	free_initialized_dongles(dongles, index);
	if (stage >= 2)
		pthread_cond_destroy(&dongles[index].cond);
	if (stage >= 1)
		pthread_mutex_destroy(&dongles[index].mutex);
	free(dongles);
	return (NULL);
}

static void	set_dongle_data(t_dongle *dongle, int index)
{
	dongle->dongle_id = index;
	dongle->state = DONGLE_STATE_AVAILABLE;
	dongle->cooldown_end = 0;
	dongle->pq = NULL;
}

t_dongle	*init_dongles(int *conf)
{
	t_dongle	*dongles;
	int			index;

	dongles = malloc(sizeof(t_dongle) * conf[NUM_OF_CODERS]);
	if (!dongles)
		return (NULL);
	index = 0;
	while (index < conf[NUM_OF_CODERS])
	{
		set_dongle_data(&dongles[index], index);
		if (pthread_mutex_init(&dongles[index].mutex, NULL) != 0)
			return (fail_dongle_init(dongles, index, 0));
		if (pthread_cond_init(&dongles[index].cond, NULL) != 0)
			return (fail_dongle_init(dongles, index, 1));
		dongles[index].pq = create_pq(conf[NUM_OF_CODERS]);
		if (!dongles[index].pq)
			return (fail_dongle_init(dongles, index, 2));
		index++;
	}
	return (dongles);
}
