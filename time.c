/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 17:30:30 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_current_us(void)
{
	struct timeval	time_value;

	if (gettimeofday(&time_value, NULL) != 0)
		return (0);
	return ((long long)time_value.tv_sec * 1000000 + time_value.tv_usec);
}

long long	get_elapsed_ms(long long start)
{
	return ((get_current_us() - start) / 1000);
}

struct timespec	convert_ms_to_timespec(long long milliseconds)
{
	struct timespec	time_spec;

	time_spec.tv_sec = milliseconds / 1000;
	time_spec.tv_nsec = (milliseconds % 1000) * 1000000;
	return (time_spec);
}
