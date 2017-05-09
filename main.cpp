#include <iostream>
#include <assert.h>
#include "BenchMark.h"
//#include "ConcurrentLinkedList.h"
#include "ConcurrentList.h"

/*
void *insert_task(void *ptr) {
    ConcurrentSortedLinkedList<int> *concurrentLinkedList = (ConcurrentSortedLinkedList<int> *) ptr;
    for (int i = 10; i > 0; i--) {
        Node<int> *n = new Node<int>(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
//        printf("insert randomly %d\n", i);
    }
}

void *delete_task(void *ptr) {
    ConcurrentSortedLinkedList<int> *concurrentLinkedList = (ConcurrentSortedLinkedList<int> *) ptr;
    for (int i = 0; i < 10000; i++) {
        Node<int> *n = new Node<int>(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
    }
}

void testConcurrentSortedLinkedList() {
    ConcurrentSortedLinkedList<int> *concurrentLinkedList = new ConcurrentSortedLinkedList<int>();

    int len1 = concurrentLinkedList->to_string();
    printf("length: %d\n", len1);

    BenchMark benchMark;
    benchMark.run(1, insert_task, (void *) concurrentLinkedList);

    int len = concurrentLinkedList->to_string();
    printf("length: %d\n", len);

    Node<int> *node = concurrentLinkedList->search_from(concurrentLinkedList->head, 1);
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
    lockfreeList<int> *testList;
    int threadId;

};

void *insert_task_List(void *ptr) {
    PTR *p = (PTR *) ptr;
    for (int i = 10; i > 0; i-=2) {
        Node<int> *n = new Node<int>(i);
        p->testList->insert(i);
//        printf("insert randomly %d\n", i);
    }
}

//int count1 = 0;
//int count2 = 0;
std::atomic<unsigned int> count1;
std::atomic<unsigned int> count2;

void *test(void * p) {
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
    lockfreeList<int> *testList = new lockfreeList<int>();

    int len;
    len = testList->to_string();
    printf("length: %d\n", len);

    PTR *p = new PTR();
    p->testList = testList;

    BenchMark benchMark;
    benchMark.run(1, insert_task_List, (void *) p);
    int *count = 0;

    len = testList->to_string();
    printf("length: %d\n", len);
    Node<int> *left = new Node<int>(-99);
    int rst = testList->search_from(testList->head->next->next, 3, left)->data;
    printf("rst = %d, left = %d", rst, left->data);

}

int main() {

    testTestList();

    return 0;
}