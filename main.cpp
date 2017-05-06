#include <iostream>
#include "BenchMark.h"
#include "ConcurrentLinkedList.h"


void *insert_random_task(void *ptr) {
    ConcurrentLinkedList<int> *concurrentLinkedList = (ConcurrentLinkedList<int> *) ptr;
    for (int i = 0; i < 1000000; i++) {
        Node<int> *n = new Node<int>(i);
        concurrentLinkedList->insert(concurrentLinkedList->head, n);
//        printf("insert randomly %d\n", i);
    }
}


int main() {

    ConcurrentLinkedList<int> *concurrentLinkedList = new ConcurrentLinkedList<int>();

    BenchMark benchMark;
    benchMark.run10x(5, insert_random_task, (void *) concurrentLinkedList);

    return 0;
}