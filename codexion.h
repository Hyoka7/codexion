/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 20:58:16 by hfujisad          #+#    #+#             */
/*   Updated: 2026/08/22 17:45:31 by hfujisad         ###   ########.fr       */
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
	unsigned long long	fifo_rank;
	long long			queue_seconds;
}						t_pqnode;

typedef struct s_pq
{
	t_pqnode			**queue;
	size_t				size;
	unsigned long long	next_fifo_rank;
	int					(*cmp)(const t_pqnode *, const t_pqnode *);
}						t_pq;

typedef enum e_config
{
	NUM_OF_CODERS,
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
	DONGLE_STATE_AVAILABLE,
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
	int					compile_count;
	int					done;
	bool				has_dongles;
	bool				request_pending;
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
	pthread_t			timer_thread;
	bool				timer_created;
	bool				start_ready;
	int					ready_workers;
};

void					print_status(t_coder *coder, const char *status);
void					print_dongle_pair(t_coder *coder);
void					print_burnout(t_sim *sim, int index);
void					stop_and_wake(t_sim *sim);
void					*coder_routine(void *arg);
void					*monitor_routine(void *arg);
void					*scheduler_timer_routine(void *arg);
int						wait_for_simulation_start(t_sim *sim);
int						start_simulation(t_sim *sim, int expected_workers);
int						init_sim_data(t_sim *sim, int *conf, t_dongle *dongles,
							t_coder *coders);
void					init_all_coders(t_sim *sim, char *scheduler);

t_pq					*create_pq(int num_coders);
int						push_pq(t_pq *queue, int coder_id, long long time_data);
t_pqnode				*pop_pq(t_pq *queue);
t_pqnode				*remove_pq_at(t_pq *queue, size_t index);
void					free_priority_queue(t_pq *pq);
int						cmp_edf(const t_pqnode *a, const t_pqnode *b);
int						cmp_fifo(const t_pqnode *a, const t_pqnode *b);

int						*parser(char **argv, char **scheduler);
long long				get_current_us(void);
long long				get_elapsed_ms(long long us_start);

t_dongle				*init_dongles(int *conf);
t_coder					*init_coders(int *conf);
int						get_dongles(t_coder *coder);
int						get_scheduled_dongles(t_coder *coder);
void					lock_coder_dongles(t_coder *coder, t_dongle **first,
							t_dongle **second);
void					unlock_coder_dongles(t_dongle *first, t_dongle *second);
bool					request_is_feasible(t_coder *coder, long long now);
long long				next_scheduler_wake(t_sim *sim);
void					schedule_requests(t_sim *sim);
void					release_dongles(t_coder *coder);
bool					is_simulation_stopped(t_sim *sim);
void					update_dongle_available(long long now,
							t_dongle *dongle);
int						try_to_push(t_dongle *dongle, t_coder *coder,
							long long data);
long long				get_time_data(t_coder *coder);
bool					is_dongle_for_me(t_coder *coder, t_dongle *dongle);
void					lock_dongles(t_dongle *first, t_dongle *second);
void					unlock_dongles(t_dongle *first, t_dongle *second);
int						sequence_get_dongles(t_coder *coder, t_dongle *first,
							t_dongle *second);

int						mainloop(int *conf, t_dongle *dongles, t_coder *coders,
							char *scheduler);

#endif
