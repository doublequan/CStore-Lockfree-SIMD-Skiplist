#include <iostream>
#include <assert.h>
#include <map>
#include "BenchMark.h"
//#include "ConcurrentLinkedList.h"
#include "ConcurrentList.h"
//#include "Skiplist.h"
#include "Benchmark/lock_free_skip_list.h"
#include "Benchmark/skip_list.h"


#define THREAD_GET_COUNT (320000 / THREAD_NUM)
#define THREAD_NUM 32
#define READ_RATIO 100
#define INSERT_RATIO 0


void *simd_skiplist_run(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockfreeList *list = (LockfreeList *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0, INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0, 99);
    for (int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if (type < READ_RATIO) {
            list->find(key);
        } else if (type < INSERT_RATIO) {
            list->insert(key, key);
        } else {
            list->remove(key);
        }
    }
}


void *baseline_lockfree_skiplist(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    LockFreeSkipList<int> *list = (LockFreeSkipList<int> *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0, INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0, 99);

    std::vector<SL_Node<int> *> preds(1000000);
    std::vector<SL_Node<int> *> succs(1000000);
    for (int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if (type < READ_RATIO) {
            list->find(key, preds, succs);
        } else if (type < INSERT_RATIO) {
            list->insert(key);
        } else {
            list->remove(key);
        }
    }
}

void *baseline_serial_skiplist(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    goodliffe::skip_list<int> *list = (goodliffe::skip_list<int> *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0, INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0, 99);

    for (int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if (type < READ_RATIO) {
            list->find(key);
        } else if (type < INSERT_RATIO) {
            list->insert(key);
        } else {
            list->erase(key);
        }
    }
}

void *baseline_map(void *ptr) {
    Param *p = (Param *) ptr;
    int thread_id = p->thread_id;
    int thread_num = p->thread_num;
    std::map<int, int> *map = (std::map<int, int> *) p->ptr;

    static thread_local std::mt19937 generator(time(0) + thread_id);
    std::uniform_int_distribution<int> distribution(0, INIT_SET_SIZE);
    std::uniform_int_distribution<int> distribution2(0, 99);

    for (int i = 0; i < THREAD_GET_COUNT; i++) {
        int key = distribution(generator);
        int type = distribution2(generator);
        if (type < READ_RATIO) {
            map->find(key);
        } else if (type < INSERT_RATIO) {
            map->insert(key, key);
        } else {
            map->erase(key);
        }
    }
}

void test_simd_skiplist() {
    LockfreeList *list = new LockfreeList();

    BenchMark benchMark;

    benchMark.run3x(THREAD_NUM, simd_skiplist_run, (void *) list);
}

void test_baseline_lock_free_skiplist() {

    LockFreeSkipList<int> *list = new LockFreeSkipList<int>(1, INIT_SET_SIZE);

    for (int i = 2; i < INIT_SET_SIZE; i++) {
        list->insert(i);
    }

    BenchMark benchMark;
    benchMark.run3x(THREAD_NUM, baseline_lockfree_skiplist, (void *) list);

}

void test_baseline_serial_skiplist() {

    goodliffe::skip_list<int> *list = new goodliffe::skip_list<int>();

    for (int i = 1; i < INIT_SET_SIZE; i++) {
        list->insert(i);
    }

    BenchMark benchMark;
    benchMark.run3x(THREAD_NUM, baseline_serial_skiplist, (void *) list);

}

void test_map() {

    std::map<int, int> *map = new std::map<int, int>();

    for (int i = 1; i < INIT_SET_SIZE; i++) {
        map->insert(i, i);
    }

    BenchMark benchMark;
    benchMark.run3x(THREAD_NUM, baseline_map, (void *) map);

}


int main() {


    printf("CStore: \n");
    test_simd_skiplist();

    printf("Baseline serial Skiplist: \n");
    test_baseline_serial_skiplist();

    printf("Baseline Lockfree SkipList: \n");
    test_baseline_lock_free_skiplist();

    printf("Std Map: \n");
    test_map();

    return 0;
}