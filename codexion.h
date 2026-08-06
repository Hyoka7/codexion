/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 20:58:16 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/06 14:28:47 by hfujisad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdbool.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

# define SUCCESS 0
# define FAILURE 1

typedef enum e_simstate
{
	IN_PROGRESS,
	COMPLETED,
	BURN_OUT,
	INTERNAL_ERROR,
	SIMSTATE_MAX
}						t_simstate;

typedef struct s_pqnode
{
	int					coder_id;
	int					fifo_rank;
	long long			queue_seconds;
}						t_pqnode;

typedef struct s_pq
{
	t_pqnode			**queue;
	size_t				size;
	int					next_fifo_rank;
	int					(*cmp)(const t_pqnode *, const t_pqnode *);
}						t_pq;

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
}						t_config;

typedef enum e_dongle_state
{
	DONGLE_STATE_AVAILABLE = 0,
	DONGLE_STATE_COOLDOWN,
	DONGLE_STATE_USING,
	DONGLE_MAX
}						t_dongle_state;

typedef struct s_dongle
{
	int					dongle_id;
	t_dongle_state		state;
	long long			cooldown_end;
	t_pq				*pq;

	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
}						t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int					coder_id;
	pthread_t			thread_id;

	long long			last_compile_start;
	long long			request_time;
	int					compile_count;
	int					done;

	t_dongle			*dongle_l;
	t_dongle			*dongle_r;

	int					*conf;
	int					is_edf;
	int					(*cmp)(const t_pqnode *, const t_pqnode *);
	t_sim				*sim;
}						t_coder;

struct					s_sim
{
	int					*conf;
	long long			start_time;
	t_simstate			simstate;
	t_coder				*coders;
	t_dongle			*dongles;
	pthread_mutex_t		state_mutex;
	pthread_cond_t		state_cond;
	pthread_mutex_t		log_mutex;
};

void					print_status(t_coder *coder, const char *status);
t_pq					*create_pq(int num_coders);
int						push_pq(t_pq *queue, int coder_id, long long time_data);
t_pqnode				*pop_pq(t_pq *q);
void					free_priority_queue(t_pq *pq);
int						*parser(char **argv, char **scheduler);
long long				get_current_ms(void);
long long				get_elapsed_ms(long long ms_start);
struct timespec			convert_ms_to_timespec(long long time_in_ms);
int						cmp_edf(const t_pqnode *a, const t_pqnode *b);
int						cmp_fifo(const t_pqnode *a, const t_pqnode *b);
int						get_single_dongle(t_dongle *dongle, t_coder *coder);
int						get_dongles(t_coder *coder);
void					release_dongles(t_coder *coder);
void					release_single_dongle(t_dongle *dongle, int *conf);
t_dongle				*init_dongles(int *conf);
t_coder					*init_coders(int *conf);
int						mainloop(int *conf, t_dongle *dongles, t_coder *coders,
							char *scheduler);
#endif