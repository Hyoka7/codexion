#include "codexion.h"
#include <stdio.h>

void	print_status(t_coder *coder, const char *status)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	if (sim->simstate == IN_PROGRESS)
		printf("%lld %d %s\n", get_elapsed_ms(sim->start_time), coder->coder_id,
			status);
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_unlock(&sim->log_mutex);
}

static int	sim_sleep(t_sim *sim, long long duration)
{
	long long	end;
	int			simstate;

	end = get_current_ms() + duration;
	while (get_current_ms() < end)
	{
		pthread_mutex_lock(&sim->state_mutex);
		simstate = sim->simstate;
		pthread_mutex_unlock(&sim->state_mutex);
		if (simstate != IN_PROGRESS)
			return (0);
		usleep(500);
	}
	return (1);
}

static void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (coder->compile_count < coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
	{
		if (get_dongles(coder) == FAILURE)
			return (NULL);
		pthread_mutex_lock(&coder->sim->state_mutex);
		coder->last_compile_start = get_current_ms();
		pthread_cond_broadcast(&coder->sim->state_cond);
		pthread_mutex_unlock(&coder->sim->state_mutex);
		print_status(coder, "is compiling");
		if (!sim_sleep(coder->sim, coder->conf[TIME_TO_COMPILE_MS]))
			return (release_dongles(coder), NULL);
		pthread_mutex_lock(&coder->sim->state_mutex);
		coder->compile_count++;
		if (coder->compile_count == coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
			coder->done = 1;
		pthread_cond_broadcast(&coder->sim->state_cond);
		pthread_mutex_unlock(&coder->sim->state_mutex);
		release_dongles(coder);
		if (coder->done)
			break ;
		print_status(coder, "is debugging");
		if (!sim_sleep(coder->sim, coder->conf[TIME_TO_DEBUG_MS]))
			return (NULL);
		print_status(coder, "is refactoring");
		if (!sim_sleep(coder->sim, coder->conf[TIME_TO_REFACTOR_MS]))
			return (NULL);
	}
	return (NULL);
}

static void	stop_and_wake(t_sim *sim)
{
	int	i;

	pthread_cond_broadcast(&sim->state_cond);
	i = 0;
	while (i < sim->conf[NUM_OF_CODERS])
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static void	print_burnout(t_sim *sim, int i)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%lld %d burned out\n", get_elapsed_ms(sim->start_time),
		sim->coders[i].coder_id);
	pthread_mutex_unlock(&sim->log_mutex);
}

static void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	long long	now;
	int			i;
	int			all_done;

	sim = (t_sim *)arg;
	while (1)
	{
		pthread_mutex_lock(&sim->state_mutex);
		if (sim->simstate != IN_PROGRESS)
		{
			pthread_mutex_unlock(&sim->state_mutex);
			return (stop_and_wake(sim), NULL);
		}
		now = get_current_ms();
		i = 0;
		all_done = 1;
		while (i < sim->conf[NUM_OF_CODERS])
		{
			if (!sim->coders[i].done)
				all_done = 0;
			if (!sim->coders[i].done && now >= sim->coders[i].last_compile_start
				+ sim->conf[TIME_TO_BURNOUT_MS])
				break ;
			i++;
		}
		if (i < sim->conf[NUM_OF_CODERS] || all_done)
		{
			sim->simstate = COMPLETED;
			if (!all_done)
			{
				sim->simstate = BURN_OUT;
				pthread_mutex_unlock(&sim->state_mutex);
				print_burnout(sim, i);
			}
			else
				pthread_mutex_unlock(&sim->state_mutex);
			return (stop_and_wake(sim), NULL);
		}
		else
			pthread_mutex_unlock(&sim->state_mutex);
		usleep(500);
	}
}

static int	init_sim(t_sim *sim, int *conf, t_dongle *dongles, t_coder *coders)
{
	memset(sim, 0, sizeof(*sim));
	sim->conf = conf;
	sim->start_time = get_current_ms();
	sim->coders = coders;
	sim->dongles = dongles;
	if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->state_cond, NULL) != 0)
		return (pthread_mutex_destroy(&sim->state_mutex), 0);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (pthread_cond_destroy(&sim->state_cond),
			pthread_mutex_destroy(&sim->state_mutex), 0);
	return (1);
}

static void	init_coder_data(t_sim *sim, char *scheduler)
{
	int	i;
	int	is_edf;

	is_edf = (strcmp(scheduler, "edf") == 0);
	i = 0;
	while (i < sim->conf[NUM_OF_CODERS])
	{
		sim->coders[i].coder_id = i + 1;
		sim->coders[i].request_time = sim->start_time;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].conf = sim->conf;
		sim->coders[i].is_edf = is_edf;
		sim->coders[i].cmp = is_edf ? cmp_edf : cmp_fifo;
		sim->coders[i].sim = sim;
		sim->coders[i].dongle_l = &sim->dongles[i];
		sim->coders[i].dongle_r = &sim->dongles[(i + 1)
			% sim->conf[NUM_OF_CODERS]];
		i++;
	}
}

int	mainloop(int *conf, t_dongle *dongles, t_coder *coders, char *scheduler)
{
	t_sim		sim;
	pthread_t	monitor;
	int			i;
	int			simres;
	int			monitor_created;

	if (!init_sim(&sim, conf, dongles, coders))
		return (1);
	init_coder_data(&sim, scheduler);
	i = 0;
	while (i < conf[NUM_OF_CODERS])
	{
		if (pthread_create(&coders[i].thread_id, NULL, coder_routine,
				&coders[i]) != 0)
			break ;
		i++;
	}
	monitor_created = (i == conf[NUM_OF_CODERS] && pthread_create(&monitor,
				NULL, monitor_routine, &sim) == 0);
	if (!monitor_created)
	{
		pthread_mutex_lock(&sim.state_mutex);
		sim.simstate = INTERNAL_ERROR;
		pthread_mutex_unlock(&sim.state_mutex);
		stop_and_wake(&sim);
	}
	else
		pthread_join(monitor, NULL);
	while (i-- > 0)
		pthread_join(coders[i].thread_id, NULL);
	pthread_mutex_lock(&sim.state_mutex);
	if (sim.simstate == INTERNAL_ERROR)
		simres = FAILURE;
	else
		simres = SUCCESS;
	pthread_mutex_unlock(&sim.state_mutex);
	pthread_mutex_destroy(&sim.log_mutex);
	pthread_cond_destroy(&sim.state_cond);
	pthread_mutex_destroy(&sim.state_mutex);
	return (simres);
}
