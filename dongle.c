/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 18:23:50 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/07 18:59:26 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long long	ft_min(long long a, long long b)
{
	if (a > b)
		return (b);
	return (a);
}

static t_dongle	*free_dongles(t_dongle *dongles, int size)
{
	int	idx;

	if (!dongles)
		return (NULL);
	idx = 0;
	while (idx < size)
	{
		pthread_mutex_destroy(&dongles[idx].mutex);
		pthread_cond_destroy(&dongles[idx].cond);
		idx++;
	}
	free(dongles);
	return (NULL);
}

t_dongle	*init_dongles(int *conf)
{
	t_dongle	*dongles;
	int			idx;

	dongles = malloc(sizeof(t_dongle) * conf[NUM_OF_CODERS]);
	if (!dongles)
		return (NULL);
	idx = 0;
	while (idx < conf[NUM_OF_CODERS])
	{
		dongles[idx].dongle_id = idx;
		dongles[idx].state = DONGLE_STATE_AVAILABLE;
		dongles[idx].cooldown_end = 0;
		if (pthread_mutex_init(&dongles[idx].mutex, NULL) != 0
			|| pthread_cond_init(&dongles[idx].cond, NULL) != 0)
			return (free_dongles(dongles, idx));
		dongles[idx].pq = create_pq(conf[NUM_OF_CODERS]);
		if (!dongles[idx].pq)
			return (free_dongles(dongles, idx));
		idx++;
	}
	return (dongles);
}

void	get_single_dongle(t_dongle *dongle, t_coder *coder)
{
	long long		time_data;
	long long		burnout_time;
	long long		wake_time;
	struct timespec	ts;

	if (coder->is_edf)
		time_data = coder->last_complie_start + coder->conf[TIME_TO_BURNOUT_MS];
	else
		time_data = get_elapsed_ms(coder->request_time);
	pthread_mutex_lock(&dongle->mutex);
	dongle->pq->cmp = coder->cmp;
	push_pq(dongle->pq, coder->coder_id, time_data);
	burnout_time = coder->last_complie_start + coder->conf[TIME_TO_BURNOUT_MS];
	while (1)
	{
		if (get_elapsed_ms(coder->request_time) >= burnout_time)
		{
			print_status(coder, "is burned out");
			exit(1);
		}
		if (dongle->state == DONGLE_STATE_COOLDOWN
			&& get_current_ms() >= dongle->cooldown_end)
			dongle->state = DONGLE_STATE_AVAILABLE;
		if (dongle->pq->queue[0]->coder_id == coder->coder_id
			&& dongle->state == DONGLE_STATE_AVAILABLE)
			break ;
		if (dongle->state == DONGLE_STATE_COOLDOWN)
		{
			wake_time = ft_min(dongle->cooldown_end, burnout_time);
			ts = convert_ms_to_timespec(wake_time);
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		else
		{
			ts = convert_ms_to_timespec(burnout_time);
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
	}
	dongle->state = DONGLE_STATE_USING;
	free(pop_pq(dongle->pq));
	pthread_mutex_unlock(&dongle->mutex);
}

void	get_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->dongle_l->dongle_id < coder->dongle_r->dongle_id)
	{
		first = coder->dongle_l;
		second = coder->dongle_r;
	}
	else
	{
		first = coder->dongle_r;
		second = coder->dongle_l;
	}
	get_single_dongle(first, coder);
	get_single_dongle(second, coder);
}

void	release_single_dongle(t_dongle *dongle, int *conf)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->state = DONGLE_STATE_COOLDOWN;
	dongle->cooldown_end = get_current_ms() + conf[DONGLE_COOLDOWN_MS];
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
void	release_dongles(t_coder *coder)
{
	release_single_dongle(coder->dongle_l, coder->conf);
	release_single_dongle(coder->dongle_r, coder->conf);
}