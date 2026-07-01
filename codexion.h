/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 20:58:16 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/01 20:34:22 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct s_pqnode
{
	int				coder_id;
	struct timeval	time_data;
}	t_pqnode;

typedef struct s_priority_queue
{
	t_pqnode	**queue;
	size_t		size;
	int			(*cmp)(const t_pqnode *, const t_pqnode *);
}	t_pq;

typedef enum e_config
{
	NUM_OF_CODERS = 0,
	TIME_TO_BURNOUT,
	TIME_TO_COMPILE,
	TIME_TO_DEBUG,
	TIME_TO_REFACTOR,
	NUMBERS_OF_COMPILES_REQUIRED,
	DONGLE_COOLDOWN,
	CONFIG_MAX
}	t_config;

t_pq		*create_pq(int num_coders);
void		push_pq(t_pq *q, int coder_id, struct timeval time_data);
t_pqnode	*pop_pq(t_pq *q);
void		free_priority_queue(t_pq *pq);
