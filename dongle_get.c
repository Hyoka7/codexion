/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_get.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	rank_dongles(t_dongle **first, t_dongle **second)
{
	t_dongle	*temporary;

	if ((*first)->dongle_id > (*second)->dongle_id)
	{
		temporary = *first;
		*first = *second;
		*second = temporary;
	}
}

int	get_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->dongle_l;
	second = coder->dongle_r;
	rank_dongles(&first, &second);
	if (first == second)
		return (sequence_get_dongles(coder, first, second));
	return (get_scheduled_dongles(coder));
}
