//
// Created by kenneth on 4/18/17.
//

#ifndef CSTORE_SKIPLIST_H
#define CSTORE_SKIPLIST_H

#include <string>
#include <vector>
#include "concurrentLinkedList.h"

#define MAX_HEIGHT 20
#define VECTOR_SIZE 32

template <class T>
class IndexNode{
private:
    T indexes[];
public:
    IndexNode* next;
    IndexNode(){
        indexes = new T[VECTOR_SIZE];
        next = NULL;
    }

    // give an int(representing the compare result), return the pointer to
    // next node to find(could be index node or storage node)
    void* routeToNextNode();
};

template <class T>
class IndexLayer{
private:
    IndexNode<T> starts[];
    IndexNode<T> ends[];
public:
    IndexLayer(){
        starts = new IndexNode[MAX_HEIGHT];
        ends = new IndexNode[MAX_HEIGHT];
        for(int i = 0; i < MAX_HEIGHT; i++){
            starts[i].next = &ends[i];
        }
    }

    // give a key, return the starting storage node pointer
    Node* find(T key);
    // give start node of storage layer, rebuild the index layer
    void rebuild(Node* start);
};
#endif //CSTORE_SKIPLIST_H
