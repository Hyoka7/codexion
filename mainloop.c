#include "codexion.h"
#include <stdio.h>

// ログ出力を安全に行うためのミューテックス（混ざり防止）
static pthread_mutex_t	g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

void	print_status(t_coder *coder, const char *status)
{
	pthread_mutex_lock(&g_log_mutex);
	printf("%lld %d %s\n", get_elapsed_ms(coder->request_time), coder->coder_id,
		status);
	pthread_mutex_unlock(&g_log_mutex);
}

// 各コーダースレッドが走らせる無限ループ（または指定回数ループ）
void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		compile_count;

	coder = (t_coder *)arg;
	compile_count = 0;
	// 初回の「前回のコンパイル開始時刻」を生時刻の0ms地点として同期
	coder->last_complie_start = 0;
	// 指定されたコンパイル回数に達するまでループ
	while (compile_count < coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
	{
		// 1. ドングルを2本安全に取得（ID順ソートによりデッドロックフリー）
		get_dongles(coder);
		// 2. コンパイル開始（EDF用のスコア起点として時刻を記録）
		coder->last_complie_start = get_elapsed_ms(coder->request_time);
		print_status(coder, "is compiling");
		usleep(coder->conf[TIME_TO_COMPILE_MS] * 1000);
		compile_count++;
		// 3. 使い終わったので2本とも解放（クールダウンへ移行）
		release_dongles(coder);
		// 指定回数をクリアしたら、最後のデバッグとリファクタリングはスキップして終了
		if (compile_count == coder->conf[NUMBERS_OF_COMPILES_REQUIRED])
			break ;
		// 4. デバッグフェーズ
		print_status(coder, "is debugging");
		usleep(coder->conf[TIME_TO_DEBUG_MS] * 1000);
		// 5. リファクタリングフェーズ
		print_status(coder, "is refactoring");
		usleep(coder->conf[TIME_TO_REFACTOR_MS] * 1000);
	}
	return (NULL);
}

int	mainloop(int *conf, t_dongle *dongles, t_coder *coders, char *scheduler)
{
	int i;
	long long start_time;
	int is_edf;

	start_time = get_current_ms();
	is_edf = (strcmp(scheduler, "edf") == 0);
	i = 0;

	// 1. 全コーダーの構造体にデータを配り、円卓状にドングルを割り当てる
	while (i < conf[NUM_OF_CODERS])
	{
		coders[i].coder_id = i + 1;
		coders[i].request_time = start_time;
		coders[i].conf = conf;
		coders[i].is_edf = is_edf;
		coders[i].cmp = is_edf ? cmp_edf : cmp_fifo;

		// 円卓の割り当て（左は自分と同じID、右は隣のID。最後の人だけ0番に戻る）
		coders[i].dongle_l = &dongles[i];
		coders[i].dongle_r = &dongles[(i + 1) % conf[NUM_OF_CODERS]];
		i++;
	}

	// 2. 一斉にスレッドを起動
	i = 0;
	while (i < conf[NUM_OF_CODERS])
	{
		if (pthread_create(&coders[i].thread_id, NULL, coder_routine,
				&coders[i]) != 0)
			return (1);
		i++;
	}

	// 3. 全員が作業を終えるのを待つ
	i = 0;
	while (i < conf[NUM_OF_CODERS])
	{
		pthread_join(coders[i].thread_id, NULL);
		i++;
	}
	return (0);
}