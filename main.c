/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 21:16:36 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/01 20:34:23 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "codexion.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/time.h>
// #include <unistd.h>

// int	compare_time(const t_pqnode *a, const t_pqnode *b)
// {
// 	if (a->time_data.tv_sec < b->time_data.tv_sec)
// 		return (-1);
// 	if (a->time_data.tv_sec > b->time_data.tv_sec)
// 		return (1);
// 	if (a->time_data.tv_usec < b->time_data.tv_usec)
// 		return (-1);
// 	if (a->time_data.tv_usec > b->time_data.tv_usec)
// 		return (1);
// 	return (0);
// }

// int	main(void)
// {
// 	struct timeval	tv;
// 	t_pq			*q = create_pq(10);

// 	q->cmp = compare_time;
// 	gettimeofday(&tv, NULL);
// 	push_pq(q, 101, tv);
// 	usleep(100);
// 	gettimeofday(&tv, NULL);
// 	push_pq(q, 102, tv);
// 	usleep(100);
// 	gettimeofday(&tv, NULL);
// 	push_pq(q, 103, tv);
// 	int	i = 0;

// 	while (i < 3)
// 	{
// 		t_pqnode	*node = pop_pq(q);

// 		if (node)
// 		{
// 			printf("Coder ID: %d, Time: %ld.%06d\n",
// 				node->coder_id,
// 			(long) node->time_data.tv_sec,
// 			(int) node->time_data.tv_usec);
// 			free(node);
// 		}
// 		i++;
// 	}
// 	free(q->queue);
// 	free(q);
// 	return (0);
// }
