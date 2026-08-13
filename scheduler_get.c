/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_get.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_for_scheduler(t_sim *sim)
{
	int				result;

	result = pthread_cond_wait(&sim->state_cond, &sim->state_mutex);
	if (result != SUCCESS)
		sim->simstate = INTERNAL_ERROR;
	return (result);
}

static int	finish_scheduled_get(t_coder *coder)
{
	if (coder->sim->simstate != IN_PROGRESS)
	{
		pthread_mutex_unlock(&coder->sim->state_mutex);
		release_dongles(coder);
		return (FAILURE);
	}
	pthread_mutex_unlock(&coder->sim->state_mutex);
	print_dongle_pair(coder);
	return (SUCCESS);
}

static void	remove_coder_request(t_pq *queue, int coder_id)
{
	size_t	index;

	index = 0;
	while (index < queue->size)
	{
		if (queue->queue[index]->coder_id == coder_id)
		{
			free(remove_pq_at(queue, index));
			return ;
		}
		index++;
	}
}

static int	enqueue_request(t_coder *coder)
{
	long long	priority;
	t_dongle	*first;
	t_dongle	*second;

	priority = get_time_data(coder);
	lock_coder_dongles(coder, &first, &second);
	if (push_pq(first->pq, coder->coder_id, priority) == FAILURE
		|| push_pq(second->pq, coder->coder_id, priority) == FAILURE)
	{
		remove_coder_request(first->pq, coder->coder_id);
		remove_coder_request(second->pq, coder->coder_id);
		unlock_coder_dongles(first, second);
		return (FAILURE);
	}
	coder->request_pending = true;
	unlock_coder_dongles(first, second);
	return (SUCCESS);
}

int	get_scheduled_dongles(t_coder *coder)
{
	int	wait_result;

	pthread_mutex_lock(&coder->sim->state_mutex);
	if (enqueue_request(coder) == FAILURE)
	{
		coder->sim->simstate = INTERNAL_ERROR;
		return (pthread_mutex_unlock(&coder->sim->state_mutex), FAILURE);
	}
	while (1)
	{
		schedule_requests(coder->sim);
		if (coder->has_dongles)
			return (finish_scheduled_get(coder));
		if (coder->sim->simstate != IN_PROGRESS)
			return (pthread_mutex_unlock(&coder->sim->state_mutex), FAILURE);
		wait_result = wait_for_scheduler(coder->sim);
		if (wait_result != SUCCESS)
			return (pthread_mutex_unlock(&coder->sim->state_mutex), FAILURE);
	}
}
