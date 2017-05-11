#include <iostream>
#include <assert.h>
#include "BenchMark.h"
//#include "ConcurrentLinkedList.h"
#include "ConcurrentList.h"
//#include "Skiplist.h"
#include "lock_free_skip_list.h"

#define THREAD_GET_COUNT (320000 / THREAD_NUM)
#define THREAD_NUM 32
#define READ_RATIO 100
#define INSERT_RATIO 0


void *insert_task_List(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;
    for (int i = thread_id; i < INIT_SET_SIZE; i += thread_num) {
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
    for (int i = thread_id; i < INIT_SET_SIZE; i += thread_num) {
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
    for (int i = thread_id; i < INIT_SET_SIZE; i += thread_num) {
//        printf("[%d] removing key %d \n", thread_id, i);
        list->remove(i);
    }
//    for (int i = 0; i < 1000000; i++) {
//        unsigned int n = list->intRand(0, 1000000);
//        list->remove(n);
//    }
}


void *testrun(void* ptr){
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0,INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0,99);
    for(int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if(type < READ_RATIO){
            list->find(key);
        }else if(type < INSERT_RATIO){
            list->insert(key, key);
        }else{
            list->remove(key);
        }
    }
}


void *testrun_skiplist(void* ptr){
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockFreeSkipList<int> *list = (LockFreeSkipList<int> *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0,INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0,99);

    std::vector < SL_Node<int> * > preds(1000000);
    std::vector < SL_Node<int> * > succs(1000000);
    for(int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if(type < READ_RATIO){
            list->find(key, preds, succs);
        }else if(type < INSERT_RATIO){
            list->insert(key);
        }else{
            list->remove(key);
        }
    }
}

void testTestList() {
    LockfreeList *list = new LockfreeList();

    BenchMark benchMark;
//    benchMark.run(THREAD_NUM, insert_task_List, (void *) list);

//    printf("after insert\n");
//
//    benchMark.run(THREAD_NUM, read_task_List, (void *) list);
//
//    benchMark.run(THREAD_NUM, delete_task, (void *) list);

    benchMark.run3x(THREAD_NUM, testrun, (void *) list);

}

void testLFSkipList() {

    LockFreeSkipList<int> *list = new LockFreeSkipList<int>(1, INIT_SET_SIZE);

    for (int i = 2; i < INIT_SET_SIZE; i++) {
        list->insert(i);
    }

    BenchMark benchMark;
    benchMark.run3x(THREAD_NUM, testrun_skiplist, (void *) list);

}

int main() {

    printf("CStore: \n");
    testTestList();

    printf("Lockfree SkipList: \n");
    testLFSkipList();

    return 0;
}