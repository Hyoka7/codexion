/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 17:30:30 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/07 18:26:53 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_current_ms(void)
{
	struct timeval	tv;
	long long		ms;

	if (gettimeofday(&tv, NULL) != 0)
		return (0);
	ms = ((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	return (ms);
}

long long	get_elapsed_ms(long long ms_start)
{
	long long	ms_now;

	ms_now = get_current_ms();
	return (ms_now - ms_start);
}

struct timespec	convert_ms_to_timespec(long long time_in_ms)
{
	struct timespec ts;

	ts.tv_sec = time_in_ms / 1000;
	ts.tv_nsec = (time_in_ms % 1000) * 1000000;
	return (ts);
}