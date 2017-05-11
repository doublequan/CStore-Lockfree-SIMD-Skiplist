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

#define TEST_SET_SIZE 10000
#define THREAD_NUM 32


void *insert_task_List(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;
    for (int i = thread_id; i < TEST_SET_SIZE; i += thread_num) {
        list->insert(i, i);
    }
//    for (int i = 0; i < 1000000; i++) {
//        unsigned int n = list->intRand(0, 1000000);
//        list->insert(n, n);
//    }
}

void *read_task_List(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;
    for (int i = thread_id; i < TEST_SET_SIZE; i += thread_num) {
        list->find(i);
    }
//    for (int i = 0; i < 1000000; i++) {
//        unsigned int n = list->intRand(0, 1000000);
//        list->insert(n, n);
//    }
}


void *delete_task(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;
    for (int i = thread_id; i < TEST_SET_SIZE; i += thread_num) {
//        printf("[%d] removing key %d \n", thread_id, i);
        list->remove(i);
    }
//    for (int i = 0; i < 1000000; i++) {
//        unsigned int n = list->intRand(0, 1000000);
//        list->remove(n);
//    }
}


//void *testrun(void* ptr){
//    Param *p = (Param *) ptr;
//    int thread_id = p->thread_id;
//    int thread_num = p->thread_num;
//    static thread_local std::mt19937 generator(time(0));
//    std::uniform_int_distribution<int> distribution(0,TEST_SET_SIZE);
//    std::uniform_int_distribution<int> distribution2(0,10);
//    // double totalIndexTime = 0;
//    // double totalStorageTime = 0;
//    for(int i = 0; i < THREAD_GET_COUNT; i++) {
//        int key = distribution(generator);
//        int type = distribution2(generator);
//
//        // double indexStartTime = CycleTimer::currentSeconds();
////    printf("key: %d", key);
//        Node *n = layer->find(key);
//        // double indexEndTime = CycleTimer::currentSeconds();
//        // totalIndexTime += indexEndTime - indexStartTime;
////    printf("find %d, return node %d\n", key, n->key);
//        Node *tmp;
//        // double storageStartTime = CycleTimer::currentSeconds();
//        if(n->key != key) {
//            Node *result = list->search_after(n, key, tmp);
//            if (result->key != key) {
//                printf("find error!");
//            };
//        }
//        // double storageEndTime = CycleTimer::currentSeconds();
//        // totalStorageTime += storageEndTime - storageStartTime;
//    }
//    // printf("time compare %f\n", totalIndexTime/totalStorageTime);
//}


void testTestList() {
    LockfreeList *list = new LockfreeList();

    BenchMark benchMark;
    benchMark.run(THREAD_NUM, insert_task_List, (void *) list);

    printf("after insert\n");

    benchMark.run(THREAD_NUM, read_task_List, (void *) list);

    benchMark.run(THREAD_NUM, delete_task, (void *) list);


//    len = list->to_string();
//    printf("length: %d\n", len);

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