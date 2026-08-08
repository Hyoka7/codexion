/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 17:53:40 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/05 20:15:29 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int cmp_fifo(const t_pqnode *a, const t_pqnode *b)
{
	if (a->queue_seconds < b->queue_seconds)
		return (-1);
	if (a->queue_seconds > b->queue_seconds)
		return (1);
	if (a->fifo_rank < b->fifo_rank)
		return (-1);
	if (a->fifo_rank > b->fifo_rank)
		return (1);
	return (0);
}

int cmp_edf(const t_pqnode *a, const t_pqnode *b)
{
	if (a->queue_seconds < b->queue_seconds)
		return (-1);
	if (a->queue_seconds > b->queue_seconds)
		return (1);
	if (a->coder_id < b->coder_id)
		return (-1);
	if (a->coder_id > b->coder_id)
		return (1);
	return (0);
}