#include <iostream>
#include "BenchMark.h"
#include "ConcurrentLinkedList.h"


void *insert_random_task(void *ptr) {
    ConcurrentLinkedList<int> *concurrentLinkedList = (ConcurrentLinkedList<int> *) ptr;
    for (int i = 0; i < 10000; i++) {
        Node<int> *n = new Node<int>(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
//        printf("insert randomly %d\n", i);
    }
}


int main() {

    ConcurrentLinkedList<int> *concurrentLinkedList = new ConcurrentLinkedList<int>();

    int len1 = concurrentLinkedList->to_string();
    printf("length: %d\n", len1);

    BenchMark benchMark;
    benchMark.run(50, insert_random_task, (void *) concurrentLinkedList);

    int len = concurrentLinkedList->to_string();
    printf("length: %d\n", len);

    return 0;
}