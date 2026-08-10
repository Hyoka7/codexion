/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_atomic.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	both_dongles_for_me(t_coder *coder, t_dongle *first,
		t_dongle *second)
{
	return (is_dongle_for_me(coder, first)
		&& is_dongle_for_me(coder, second));
}

static void	grab_both_dongles(t_coder *coder, t_dongle *first,
		t_dongle *second)
{
	first->state = DONGLE_STATE_USING;
	second->state = DONGLE_STATE_USING;
	free(pop_pq(first->pq));
	free(pop_pq(second->pq));
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	print_status(coder, "has taken a dongle");
	print_status(coder, "has taken a dongle");
}

static int	register_requests(t_coder *coder, t_dongle *first,
		t_dongle *second)
{
	long long	time_data;

	time_data = get_time_data(coder);
	if (try_to_push(first, coder, time_data) == FAILURE
		|| try_to_push(second, coder, time_data) == FAILURE)
		return (FAILURE);
	return (SUCCESS);
}

int	atomic_get_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	long long	current_time;

	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	if (register_requests(coder, first, second) == FAILURE)
	{
		pthread_mutex_unlock(&second->mutex);
		pthread_mutex_unlock(&first->mutex);
		return (FAILURE);
	}
	while (1)
	{
		current_time = get_current_ms();
		update_dongle_state(current_time, first);
		update_dongle_state(current_time, second);
		if (both_dongles_for_me(coder, first, second))
			return (grab_both_dongles(coder, first, second), SUCCESS);
		if (wait_for_dongles(coder, first, second) == FAILURE)
			break ;
	}
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	return (FAILURE);
}
