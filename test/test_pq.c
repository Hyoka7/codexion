#include "../codexion.h"
#include <assert.h>

int test1(void)
{
    t_pqnode a;
    t_pqnode b;

    a.coder_id = 1;
    a.fifo_rank = 10;
    a.queue_seconds = 100;
    b = a;
    assert(cmp_fifo(&a, &b) == 0);
    assert(cmp_edf(&a, &b) == 0);
    return (0);
}
int test2(void)
{
    t_pqnode a;
    t_pqnode b;

    a.coder_id = 1;
    a.fifo_rank = 10;
    a.queue_seconds = 100;
    b = a;
    b.fifo_rank = 2;
    assert(cmp_fifo(&a, &b) > 0);
    assert(cmp_fifo(&b, &a) < 0);
    return (0);
}
int test3(void)
{
    t_pqnode a;
    t_pqnode b;

    a.coder_id = 1;
    a.fifo_rank = 10;
    a.queue_seconds = 100;
    b = a;
    b.coder_id = 2;
    assert(cmp_edf(&a, &b) < 0);
    assert(cmp_edf(&b, &a) > 0);
    return (0);
}
int test4()
{
    t_pq *pq;

    pq = create_pq(3);
    assert(pq != NULL);
    pq->cmp = cmp_edf;
    push_pq(pq, 2, 1);
    push_pq(pq, 3, 1);
    push_pq(pq, 1, 1);
    for (int i = 1; i <= 3; i++)
    {
        t_pqnode *node = pop_pq(pq);
        assert(node != NULL);
        assert(node->coder_id == i);
        free(node);
    }
    assert(pop_pq(pq) == NULL);
    free_priority_queue(pq);
    return (0);
}
int test5()
{
    t_pq *pq = create_pq(2);
    assert(pq != NULL);
    pq->cmp = cmp_edf;
    assert(push_pq(pq, 1, 1) == SUCCESS);
    assert(push_pq(pq, 1, 1) == SUCCESS);
    assert(pq->size == 2);
    int i = 2;
    while (i--)
    {
        t_pqnode *node = pop_pq(pq);
        assert(node != NULL);
        assert(node->coder_id == 1);
        free(node);
    }
    assert(pop_pq(pq) == NULL);
    free_priority_queue(pq);
    return (0);
}
int main(void)
{
    test1();
    test2();
    test3();
    test4();
    test5();
}