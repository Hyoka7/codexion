/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 20:58:16 by hfujisad          #+#    #+#             */
/*   Updated: 2026/07/07 18:27:17 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct s_pqnode
{
	int				coder_id;
	long long		queue_seconds;
}					t_pqnode;

typedef struct s_priority_queue
{
	t_pqnode		**queue;
	size_t			size;
	int				(*cmp)(const t_pqnode *, const t_pqnode *);
}					t_pq;

typedef enum e_config
{
	NUM_OF_CODERS = 0,
	TIME_TO_BURNOUT_MS,
	TIME_TO_COMPILE_MS,
	TIME_TO_DEBUG_MS,
	TIME_TO_REFACTOR_MS,
	NUMBERS_OF_COMPILES_REQUIRED,
	DONGLE_COOLDOWN_MS,
	CONFIG_MAX
}					t_config;

typedef enum e_dongle_state
{
	DONGLE_STATE_AVAILABLE = 0,
	DONGLE_STATE_COOLDOWN,
	DONGLE_STATE_USING,
	DONGLE_MAX
}					t_dongle_state;

typedef struct s_dongle
{
	int				dongle_id;
	t_dongle_state	state;
	long long		cooldown_end;
	t_pq			*pq;

	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
}					t_dongle;

typedef struct s_coder
{
	int				coder_id;
	pthread_t		thread_id;

	long long		last_complie_start;
	long long		request_time;

	t_dongle		*dongle_l;
	t_dongle		*dongle_r;

	int				*conf;
	int				is_edf;
	int				(*cmp)(const t_pqnode *, const t_pqnode *);
}					t_coder;

t_pq				*create_pq(int num_coders);
void				push_pq(t_pq *queue, int coder_id, long long time_data);
t_pqnode			*pop_pq(t_pq *q);
void				free_priority_queue(t_pq *pq);
int					*parser(char **argv, char **scheduler);
long long			get_current_ms(void);
long long			get_elapsed_ms(long long ms_start);
struct timespec		convert_ms_to_timespec(long long time_in_ms);
int					cmp_edf(const t_pqnode *a, const t_pqnode *b);
int					cmp_fifo(const t_pqnode *a, const t_pqnode *b);
void				get_single_dongle(t_dongle *dongle, t_coder *coder);
void				get_dongles(t_coder *coder);
void				release_dongles(t_coder *coder);
void				release_single_dongle(t_dongle *dongle, int *conf);
t_dongle			*init_dongles(int *conf);
t_coder				*init_coders(int *conf);