/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 17:30:10 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	cleanup_all(int *conf, t_dongle *dongles, t_coder *coders)
{
	int	index;

	index = 0;
	while (dongles && conf && index < conf[NUM_OF_CODERS])
	{
		pthread_cond_destroy(&dongles[index].cond);
		pthread_mutex_destroy(&dongles[index].mutex);
		free_priority_queue(dongles[index].pq);
		index++;
	}
	free(dongles);
	free(coders);
	free(conf);
}

int	main(int argc, char *argv[])
{
	char		*scheduler;
	int			*conf;
	t_dongle	*dongles;
	t_coder		*coders;
	int			status;

	if (argc != 9)
		return (FAILURE);
	conf = parser(argv, &scheduler);
	if (!conf)
		return (FAILURE);
	dongles = init_dongles(conf);
	coders = init_coders(conf);
	if (!dongles || !coders)
		return (cleanup_all(conf, dongles, coders), FAILURE);
	status = mainloop(conf, dongles, coders, scheduler);
	if (status != INTERNAL_ERROR)
		cleanup_all(conf, dongles, coders);
	return (status);
}
