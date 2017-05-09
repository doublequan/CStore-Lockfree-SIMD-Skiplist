#include <iostream>
#include <assert.h>
#include "BenchMark.h"
//#include "ConcurrentLinkedList.h"
#include "ConcurrentList.h"

/*
void *insert_task(void *ptr) {
    ConcurrentSortedLinkedList *concurrentLinkedList = (ConcurrentSortedLinkedList *) ptr;
    for (int i = 10; i > 0; i--) {
        Node *n = new Node(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
//        printf("insert randomly %d\n", i);
    }
}

void *delete_task(void *ptr) {
    ConcurrentSortedLinkedList *concurrentLinkedList = (ConcurrentSortedLinkedList *) ptr;
    for (int i = 0; i < 10000; i++) {
        Node *n = new Node(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
    }
}

void testConcurrentSortedLinkedList() {
    ConcurrentSortedLinkedList *concurrentLinkedList = new ConcurrentSortedLinkedList();

    int len1 = concurrentLinkedList->to_string();
    printf("length: %d\n", len1);

    BenchMark benchMark;
    benchMark.run(1, insert_task, (void *) concurrentLinkedList);

    int len = concurrentLinkedList->to_string();
    printf("length: %d\n", len);

    Node *node = concurrentLinkedList->search_after(concurrentLinkedList->head, 1);
    concurrentLinkedList->mark_delete(node);

    len = concurrentLinkedList->to_string();
    printf("length: %d\n", len);

    concurrentLinkedList->garbage_collect();

    len = concurrentLinkedList->to_string();
    printf("length: %d\n", len);
}

*/

class PTR {
public:
    lockfreeList *list;
    int thread_id;
    int thread_num;
};

void *insert_task_List(void *ptr) {
    PTR *p = (PTR *) ptr;
    for (int i = 1; i < 150; i += 1) {
        Node *n = new Node(i);
        p->list->insert(i);
//        printf("insert randomly %d\n", i);
    }
}

void *delete_task(void *ptr) {
    PTR *p = (PTR *) ptr;
    for (int i = 1; i < 100; i += 1) {
        p->list->remove(i);
//        printf("insert randomly %d\n", i);
    }
}

//int count1 = 0;
//int count2 = 0;
std::atomic<unsigned int> count1;
std::atomic<unsigned int> count2;

void *test(void *p) {
//    int *count = (int *) p;
    for (int i = 0; i < 10000; i++) {
        count1++;
        count2++;

        int loop = rand() % 1000 + 1000;
        while (loop > 0) loop--;

        count2--;
        count1--;
    }
}

void testTestList() {
    lockfreeList *list = new lockfreeList();

    int len;
    len = list->to_string();
    printf("length: %d\n", len);

    PTR *p = new PTR();
    p->list = list;
    p->thread_num = 32;

    func_t funcs[] = {insert_task_List, delete_task};
    int thread_nums[] = {1, 3};
    PTR *ptrs[] = {p, p};

    BenchMark benchMark;
//    benchMark.run(32, insert_task_List, (void *) p);
//    benchMark.mix_run(thread_nums, funcs, (void **) ptrs, 2);

//    len = list->to_string();
//    printf("length: %d\n", len);

//    list->remove(999);
//    list->remove(998);
//    list->remove(997);
//    list->remove(996);
//    list->remove(5);
//    list->remove(8);
    list->insert(50);
//    list->remove(0);
    list->remove(50);
//    list->insert(48);

//    assert(list->find(50));
//    list->remove(3);

    len = list->to_string();
    printf("length: %d\n", len);


//    Node *left = new Node(-1);
//    int rst = list->search_after(list->head, 2, left)->data;
//    printf("rst = %d, left = %d", rst, left->data);

//    assert(list->find(6));

}

int main() {

    testTestList();

    return 0;
}