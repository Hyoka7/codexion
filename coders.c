/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 18:44:52 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/07 17:54:46 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_coder	*init_coders(int *conf)
{
	t_coder	*coders;

	coders = malloc(sizeof(t_coder) * conf[NUM_OF_CODERS]);
	if (!coders)
		return (NULL);
	memset(coders, 0, sizeof(t_coder) * conf[NUM_OF_CODERS]);
	return (coders);
}
