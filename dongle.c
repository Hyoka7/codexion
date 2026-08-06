/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 18:23:50 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/06 20:58:54 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long long	ft_min(long long a, long long b)
{
	if (a > b)
		return (b);
	return (a);
}

static t_dongle	*free_dongles(t_dongle *dongles, int size, bool is_mutex,
		bool is_cond)
{
	int	idx;

	if (!dongles)
		return (NULL);
	idx = 0;
	while (idx < size)
	{
		pthread_mutex_destroy(&dongles[idx].mutex);
		pthread_cond_destroy(&dongles[idx].cond);
		free_priority_queue(dongles[idx].pq);
		idx++;
	}
	if (is_mutex)
		pthread_mutex_destroy(&dongles[idx].mutex);
	if (is_cond)
	{
		pthread_cond_destroy(&dongles[idx].cond);
		free_priority_queue(dongles[idx].pq);
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
		if (pthread_mutex_init(&dongles[idx].mutex, NULL) != 0)
			return (free_dongles(dongles, idx, false, false));
		else if (pthread_cond_init(&dongles[idx].cond, NULL) != 0)
			return (free_dongles(dongles, idx, true, false));
		dongles[idx].pq = create_pq(conf[NUM_OF_CODERS]);
		if (!dongles[idx].pq)
			return (free_dongles(dongles, idx, true, true));
		idx++;
	}
	return (dongles);
}

static int	simulation_stopped(t_sim *sim)
{
	int	simstate;

	pthread_mutex_lock(&sim->state_mutex);
	simstate = sim->simstate;
	pthread_mutex_unlock(&sim->state_mutex);
	return (simstate);
}

static void	update_dongle_state(long long now_time, t_dongle *dongle)
{
	if (dongle->cooldown_end <= now_time
		&& dongle->state == DONGLE_STATE_COOLDOWN)
		dongle->state = DONGLE_STATE_AVAILABLE;
}

static int	try_to_push(t_dongle *dongle, t_coder *coder, long long time_data)
{
	dongle->pq->cmp = coder->cmp;
	if (push_pq(dongle->pq, coder->coder_id, time_data) == FAILURE)
	{
		pthread_mutex_lock(&coder->sim->state_mutex);
		if (coder->sim->simstate == IN_PROGRESS)
			coder->sim->simstate = INTERNAL_ERROR;
		pthread_mutex_unlock(&coder->sim->state_mutex);
		return (FAILURE);
	}
	return (SUCCESS);
}

int	get_single_dongle(t_dongle *dongle, t_coder *coder)
{
	long long		time_data;
	long long		wake_time;
	struct timespec	ts;

	if (coder->is_edf)
		time_data = coder->last_compile_start + coder->conf[TIME_TO_BURNOUT_MS];
	else
		time_data = get_elapsed_ms(coder->request_time);
	pthread_mutex_lock(&dongle->mutex);
	if (try_to_push(dongle, coder, time_data) == FAILURE)
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (FAILURE);
	}
	while (1)
	{
		if (simulation_stopped(coder->sim))
		{
			pthread_mutex_unlock(&dongle->mutex);
			return (FAILURE);
		}
		update_dongle_state(get_current_ms(), dongle);
		if (dongle->pq->queue[0]->coder_id == coder->coder_id
			&& dongle->state == DONGLE_STATE_AVAILABLE)
			break ;
		if (dongle->state == DONGLE_STATE_COOLDOWN)
		{
			wake_time = ft_min(dongle->cooldown_end, get_current_ms() + 1);
			ts = convert_ms_to_timespec(wake_time);
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		else
		{
			ts = convert_ms_to_timespec(get_current_ms() + 1);
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
	}
	dongle->state = DONGLE_STATE_USING;
	print_status(coder, "has taken a dongle");
	free(pop_pq(dongle->pq));
	pthread_mutex_unlock(&dongle->mutex);
	return (SUCCESS);
}

static void	rank_dongles(t_dongle **first, t_dongle **second)
{
	t_dongle	*tmp;

	if ((*first)->dongle_id > (*second)->dongle_id)
	{
		tmp = *first;
		*first = *second;
		*second = tmp;
	}
}

static int	get_two_dongle(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (get_single_dongle(first, coder) == FAILURE)
		return (FAILURE);
	if (get_single_dongle(second, coder) == FAILURE)
	{
		release_single_dongle(first, coder->conf);
		return (FAILURE);
	}
	return (SUCCESS);
}

int	get_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->dongle_l;
	second = coder->dongle_r;
	rank_dongles(&first, &second);
	return (get_two_dongle(coder, first, second));
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
