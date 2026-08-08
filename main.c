/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 17:30:10 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/04 17:30:13 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void cleanup_all(int *conf, t_dongle *dongles, t_coder *coders)
{
	int i;

	if (dongles && conf)
	{
		i = 0;
		while (i < conf[NUM_OF_CODERS])
		{
			pthread_mutex_destroy(&dongles[i].mutex);
			pthread_cond_destroy(&dongles[i].cond);
			if (dongles[i].pq)
				free_priority_queue(dongles[i].pq);
			i++;
		}
		free(dongles);
	}
	if (coders)
		free(coders);
	if (conf)
		free(conf);
}

int main(int argc, char *argv[])
{
	char *scheduler;
	int *conf;
	t_dongle *dongles;
	t_coder *coders;
	int status;

	if (argc != 9)
		return (1);
	conf = parser(argv, &scheduler);
	if (!conf)
		return (1);
	dongles = init_dongles(conf);
	coders = init_coders(conf);
	if (!dongles || !coders)
	{
		cleanup_all(conf, dongles, coders);
		return (1);
	}
	status = mainloop(conf, dongles, coders, scheduler);
	if (status != INTERNAL_ERROR)
		cleanup_all(conf, dongles, coders);
	return (status);
}
