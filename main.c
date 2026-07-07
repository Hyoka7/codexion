/* ************************************************************************** */
/* */
/* :::      ::::::::   */
/* main.c                                             :+:      :+:    :+:   */
/* +:+ +:+         +:+     */
/* By: hfujisad <hfujisad@student.42tokyo.jp>     +#+  +:+       +#+        */
/* +#+#+#+#+#+   +#+           */
/* Created: 2026/06/29 21:16:36 by hfujisad          #+#    #+#             */
/* Updated: 2026/07/07 18:20:00 by hfujisad         ###   ########.fr       */
/* */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>

// コーダースレッドのテスト用ルーチン
void	*test_coder_routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*target_dongle;

	coder = (t_coder *)arg;
	// 今回はテスト用として、全員 ID 0 のドングルを奪い合わせる
	target_dongle = coder->dongle_l;
	printf("[%lld ms] Coder %d: Requesting Dongle %d\n",
		get_elapsed_ms(coder->request_time), coder->coder_id,
		target_dongle->dongle_id);
	// ドングルを獲得しにいく（中で push_pq され、Available になるまで待機する）
	get_single_dongle(target_dongle, coder);
	printf("[%lld ms] Coder %d: !!! ACQUIRED !!! Dongle %d\n",
		get_elapsed_ms(coder->request_time), coder->coder_id,
		target_dongle->dongle_id);
	// 獲得して300msコンパイル作業をしていると仮定してスリープ
	usleep(300 * 1000);
	printf("[%lld ms] Coder %d: Releasing Dongle %d\n",
		get_elapsed_ms(coder->request_time), coder->coder_id,
		target_dongle->dongle_id);
	// ドングルを解放する
	release_single_dongle(target_dongle, coder->conf);
	return (NULL);
}

int	main(int argc, char *argv[])
{
	char *scheduler;
	int *parse_res;
	t_dongle *dongles;
	t_coder *coders;
	long long start_time;

	if (argc != 9)
		return (printf("Usage error\n"), 1);
	parse_res = parser(argv, &scheduler);
	if (!parse_res)
		return (printf("Parse error\n"), 1);

	start_time = get_current_ms();
	dongles = init_dongles(parse_res);
	coders = init_coders(parse_res);

	// テスト用に2人のコーダーにデータを仕込む
	// コーダー1の設定
	coders[0].coder_id = 1;
	coders[0].request_time = start_time;
	coders[0].last_complie_start = 0;
	coders[0].dongle_l = &dongles[0]; // ドングルID 0番を狙わせる
	coders[0].conf = parse_res;
	coders[0].is_edf = (strcmp(scheduler, "edf") == 0);
	coders[0].cmp = coders[0].is_edf ? cmp_edf : cmp_fifo;

	// コーダー2の設定
	coders[1].coder_id = 2;
	coders[1].request_time = start_time;
	coders[1].last_complie_start = 0;
	coders[1].dongle_l = &dongles[0]; // 同じくドングルID 0番を狙わせる
	coders[1].conf = parse_res;
	coders[1].is_edf = coders[0].is_edf;
	coders[1].cmp = coders[0].cmp;

	printf("--- STARTING DONGLE ACQUISITION TEST ---\n");
	// 2つのスレッドを同時に立ち上げて競合させる
	pthread_create(&coders[0].thread_id, NULL, test_coder_routine, &coders[0]);
	usleep(50 * 1000); // わずかにズラしてコーダー2を起動
	pthread_create(&coders[1].thread_id, NULL, test_coder_routine, &coders[1]);

	// スレッドの終了を待つ
	pthread_join(coders[0].thread_id, NULL);
	pthread_join(coders[1].thread_id, NULL);
	printf("--- TEST FINISHED ---\n");

	// クリーンアップは省略（テストのため）
	return (0);
}