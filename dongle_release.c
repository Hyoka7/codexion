/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_release.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 00:00:00 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/10 00:00:00 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	release_single_dongle(t_dongle *dongle, int *conf)
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
	if (coder->dongle_l != coder->dongle_r)
		release_single_dongle(coder->dongle_r, coder->conf);
	pthread_mutex_lock(&coder->sim->state_mutex);
	coder->has_dongles = false;
	pthread_cond_broadcast(&coder->sim->state_cond);
	pthread_mutex_unlock(&coder->sim->state_mutex);
}
