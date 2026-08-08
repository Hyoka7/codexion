/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 18:23:50 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/07 21:46:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void release_single_dongle(t_dongle *dongle, int *conf);

static long long ft_max(long long a, long long b)
{
	if (a < b)
		return (b);
	return (a);
}
static long long ft_min(long long a, long long b)
{
	if (a > b)
		return (b);
	return (a);
}

static t_dongle *free_dongles(t_dongle *dongles, int size, bool is_mutex,
							  bool is_cond)
{
	int idx;

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

t_dongle *init_dongles(int *conf)
{
	t_dongle *dongles;
	int idx;

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

static int simulation_stopped(t_sim *sim)
{
	int simstate;

	pthread_mutex_lock(&sim->state_mutex);
	simstate = sim->simstate;
	pthread_mutex_unlock(&sim->state_mutex);
	return (simstate);
}

static void update_dongle_state(long long now_time, t_dongle *dongle)
{
	if (dongle->cooldown_end <= now_time && dongle->state == DONGLE_STATE_COOLDOWN)
		dongle->state = DONGLE_STATE_AVAILABLE;
}

static int try_to_push(t_dongle *dongle, t_coder *coder, long long time_data)
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
static void just_unlock(t_dongle *first, t_dongle *second)
{
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_unlock(&second->mutex);
}

static void just_lock(t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
}

static long long get_dongle_waketime(t_dongle *first, t_dongle *second)
{
	long long first_wake;
	long long second_wake;

	if (first->state == DONGLE_STATE_COOLDOWN)
		first_wake = first->cooldown_end;
	else
		first_wake = 0;
	if (second->state == DONGLE_STATE_COOLDOWN)
		second_wake = second->cooldown_end;
	else
		second_wake = 0;
	return (ft_max(first_wake, second_wake));
}

static int wait_for_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	long long waketime;
	int wait_res;
	struct timespec ts;

	waketime = get_dongle_waketime(first, second);
	pthread_mutex_lock(&coder->sim->state_mutex);
	if (coder->sim->simstate != IN_PROGRESS)
	{
		pthread_mutex_unlock(&coder->sim->state_mutex);
		return (FAILURE);
	}
	just_unlock(second, first);
	if (waketime == 0)
		wait_res = pthread_cond_wait(&coder->sim->state_cond, &coder->sim->state_mutex);
	else
	{
		ts = convert_ms_to_timespec(waketime);
		wait_res = pthread_cond_timedwait(&coder->sim->state_cond,
										  &coder->sim->state_mutex, &ts);
	}
	if (wait_res != SUCCESS && wait_res != ETIMEDOUT)
		coder->sim->simstate = INTERNAL_ERROR;
	pthread_mutex_unlock(&coder->sim->state_mutex);
	just_lock(first, second);
	if (wait_res != SUCCESS && wait_res != ETIMEDOUT)
		return (FAILURE);
	return (SUCCESS);
}

static long long get_time_data(t_coder *coder)
{
	long long time_data;

	if (coder->is_edf)
		time_data = coder->last_compile_start + coder->conf[TIME_TO_BURNOUT_MS];
	else
		time_data = get_elapsed_ms(coder->request_time);
	return (time_data);
}

static bool is_dongle_for_me(t_coder *coder, t_dongle *dongle)
{
	if (dongle->pq->size == 0)
		return (false);
	if (dongle->pq->queue[0]->coder_id == coder->coder_id && dongle->state == DONGLE_STATE_AVAILABLE)
		return (true);
	return (false);
}

static bool are_dongles_for_me(t_coder *coder, t_dongle *first,
							   t_dongle *second)
{
	if (is_dongle_for_me(coder, first) && is_dongle_for_me(coder, second))
		return (true);
	return (false);
}

static int get_single_dongle(t_dongle *dongle, t_coder *coder)
{
	long long time_data;
	long long wake_time;
	int wait_res;
	struct timespec ts;

	time_data = get_time_data(coder);
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
		if (is_dongle_for_me(coder, dongle))
			break;
		if (dongle->state == DONGLE_STATE_COOLDOWN)
		{
			wake_time = ft_min(dongle->cooldown_end, get_current_ms() + 1);
			ts = convert_ms_to_timespec(wake_time);
			wait_res = pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		else
		{
			ts = convert_ms_to_timespec(get_current_ms() + 1);
			wait_res = pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		if (wait_res != SUCCESS && wait_res != ETIMEDOUT)
		{
			pthread_mutex_lock(&coder->sim->state_mutex);
			coder->sim->simstate = INTERNAL_ERROR;
			pthread_mutex_unlock(&coder->sim->state_mutex);
			pthread_mutex_unlock(&dongle->mutex);
			return (FAILURE);
		}
	}
	dongle->state = DONGLE_STATE_USING;
	print_status(coder, "has taken a dongle");
	free(pop_pq(dongle->pq));
	pthread_mutex_unlock(&dongle->mutex);
	return (SUCCESS);
}

static void rank_dongles(t_dongle **first, t_dongle **second)
{
	t_dongle *tmp;

	if ((*first)->dongle_id > (*second)->dongle_id)
	{
		tmp = *first;
		*first = *second;
		*second = tmp;
	}
}

static int sequence_get_dongle(t_coder *coder, t_dongle *first,
							   t_dongle *second)
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

static void grab_dongle(t_dongle *dongle)
{
	dongle->state = DONGLE_STATE_USING;
	free(pop_pq(dongle->pq));
}

static int atomic_get_dongle(t_coder *coder, t_dongle *first, t_dongle *second)
{
	long long current_time;
	long long time_data;
	int wait_res;

	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	time_data = get_time_data(coder);
	if (try_to_push(first, coder, time_data) == FAILURE)
	{
		just_unlock(second, first);
		return (FAILURE);
	}
	if (try_to_push(second, coder, time_data) == FAILURE)
	{
		just_unlock(second, first);
		return (FAILURE);
	}
	while (1)
	{
		current_time = get_current_ms();
		update_dongle_state(current_time, first);
		update_dongle_state(current_time, second);
		if (are_dongles_for_me(coder, first, second))
		{
			grab_dongle(first);
			grab_dongle(second);
			just_unlock(second, first);
			print_status(coder, "has taken a dongle");
			print_status(coder, "has taken a dongle");
			return (SUCCESS);
		}
		wait_res = wait_for_dongles(coder, first, second);
		if (wait_res == FAILURE)
		{
			just_unlock(second, first);
			return (FAILURE);
		}
	}
}

int get_dongles(t_coder *coder)
{
	t_dongle *first;
	t_dongle *second;

	first = coder->dongle_l;
	second = coder->dongle_r;
	rank_dongles(&first, &second);
	if (first == second)
		return (sequence_get_dongle(coder, first, second));
	else
		return (atomic_get_dongle(coder, first, second));
}

static void release_single_dongle(t_dongle *dongle, int *conf)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->state = DONGLE_STATE_COOLDOWN;
	dongle->cooldown_end = get_current_ms() + conf[DONGLE_COOLDOWN_MS];
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
void release_dongles(t_coder *coder)
{
	release_single_dongle(coder->dongle_l, coder->conf);
	release_single_dongle(coder->dongle_r, coder->conf);
	pthread_mutex_lock(&coder->sim->state_mutex);
	pthread_cond_broadcast(&coder->sim->state_cond);
	pthread_mutex_unlock(&coder->sim->state_mutex);
}
