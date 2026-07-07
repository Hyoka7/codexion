#include "codexion.h"

static void	cleanup_all(int *conf, t_dongle *dongles, t_coder *coders)
{
	int	i;

	if (dongles && conf)
	{
		i = 0;
		while (i < conf[NUM_OF_CODERS])
		{
			pthread_mutex_destroy(&dongles[i].mutex);
			pthread_cond_destroy(&dongles[i].cond);
			if (dongles[i].pq)
				free_priority_queue(dongles[i].pq);
			i++;
		}
		free(dongles);
	}
	if (coders)
		free(coders);
	if (conf)
		free(conf);
}

int	main(int argc, char *argv[])
{
	char *scheduler;
	int *conf;
	t_dongle *dongles;
	t_coder *coders;

	if (argc != 9)
		return (1);

	// 1. 引数のパース
	conf = parser(argv, &scheduler);
	if (!conf)
		return (1);

	// 2. ドングルとコーダーのメモリ確保・初期化
	dongles = init_dongles(conf);
	coders = init_coders(conf);
	if (!dongles || !coders)
	{
		cleanup_all(conf, dongles, coders);
		return (1);
	}

	// 3. メインルーチンの実行
	mainloop(conf, dongles, coders, scheduler);

	// 4. 完全なクリーンアップをして終了（リークゼロ）
	cleanup_all(conf, dongles, coders);
	return (0);
}