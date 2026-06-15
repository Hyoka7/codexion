#include <stdio.h>
#include <pthread.h>
#include "codexion.h"

int	cnt = 0;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void *routine(void *p)
{
	for (int i = 0; i < 10000; i++)
    {
        pthread_mutex_lock(&mutex);
		cnt++;
        pthread_mutex_unlock(&mutex);
    }
	return (NULL);
}

// int main(void)
// {
// 	pthread_t p1, p2;

// 	// 2つのスレッドで並列処理する
// 	pthread_create(&p1, NULL, &routine, NULL);
// 	pthread_create(&p2, NULL, &routine, NULL);

// 	// 終了するまで待つ
// 	pthread_join(p1, NULL);
// 	pthread_join(p2, NULL);

// 	printf("cnt -> %d\n", cnt);
// }


int main()
{
    int pop_id;
    float pop_priority;

    t_priority_queue *q = create_priority_queue();
    push_priority_queue(q, 100, 0.5);
    push_priority_queue(q, 101, 0.7);
    push_priority_queue(q, 102, 0.3);
    for (int i = 0; i < 3; i++)
    {
        pop_priority_queue(q, &pop_id, &pop_priority);
        printf("%d %f\n", pop_id, pop_priority);
    }
}