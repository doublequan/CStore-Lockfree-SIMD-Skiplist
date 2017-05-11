#include <iostream>
#include <assert.h>
#include "BenchMark.h"
//#include "ConcurrentLinkedList.h"
#include "ConcurrentList.h"
//#include "Skiplist.h"

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
    LockfreeList *list;
    int thread_id;
    int thread_num;
};

void *insert_task_List(void *ptr) {
    PTR *p = (PTR *) ptr;
    for (int i = 1; i < 100000; i += 1) {
        p->list->insert(i, 0);
//        printf("insert randomly %d\n", i);
    }
}

void *delete_task(void *ptr) {
    PTR *p = (PTR *) ptr;
    for (int i = 1; i < 100000; i += 1) {
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
    LockfreeList *list = new LockfreeList();

    int len;
    len = list->to_string();
    printf("length: %d\n", len);

    PTR *p = new PTR();
    p->list = list;
//    p->thread_num = 32;

//    func_t funcs[] = {insert_task_List, delete_task};
//    int thread_nums[] = {10, 20};
//    PTR *ptrs[] = {p, p};

    BenchMark benchMark;
    benchMark.run(1, insert_task_List, (void *) p);

    printf("after insert\n");

    benchMark.run(1, delete_task, (void *) p);

//    pthread_t thread;
//    pthread_create(&thread, NULL, delete_task, (void *)p);


//    Node *cur = list->head;
//    Node *last = NULL;
//    while ((cur = get_node_address(cur->next)) != list->tail) {
//        if (last != NULL) {
//            last->next = set_confirm_delete(last->next);
//            last = NULL;
//        }
//        if (get_is_delete(cur->next)) {
//            last = cur;
//        }
////        for(int i=0; i<100000;i++);
//        usleep(1000);
//    }
//    if (last != NULL) {
//        last->next = set_confirm_delete(last->next);
//        last = NULL;
//    }

//    pthread_join(thread, NULL);


//    list->insert(990, 0);

//    benchMark.mix_run(thread_nums, funcs, (void **) ptrs, 2);

//    printf("find result %d\n", list->find(25));

    sleep(5);

    list->insert(1000000000, 1);
    Node *left = new Node(0,0,0);
    list->search_after(list->head, 100000000, left);


    len = list->to_string();
    printf("length: %d\n", len);

//    list->remove(999);
//    list->remove(998);
//    list->remove(997);
//    list->remove(996);
//    list->remove(5);
//    list->remove(8);
//    list->insert(50, 0);
//    list->insert(49, 0);
////    list->remove(1);
//    list->remove(50);
////    list->remove(8);
//    list->insert(51, 0);
//    list->insert(50, 0);
//    list->remove(50);
//    list->insert(50, 0);
//    for (int i = 10000; i < 10100; i++) {
//        list->insert(i, 1);
//    }
//    list->insert(50, 0);

//    assert(list->find(50));
//    list->remove(3);

//    len = list->to_string();
//    printf("length: %d\n", len);


//    Node *left = new Node(-1);
//    int rst = list->search_after(list->head, 2, left)->key;
//    printf("rst = %d, left = %d", rst, left->key);

//    assert(list->find(6));




}

int main() {

    testTestList();

    return 0;
}