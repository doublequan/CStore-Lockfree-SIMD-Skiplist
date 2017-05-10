//
// Created by Quan Quan on 5/10/17.
//

#ifndef CLION_CSTORE_H
#define CLION_CSTORE_H


#include "ConcurrentList.h"
#include "Skiplist.h"

class CStore {
public:
    LockfreeList *storage_layer;
    IndexLayer* index_layer;

    std::atomic<unsigned int> modification_counter;
    std::atomic<unsigned int> ongoing_query_counter;
    std::atomic<unsigned int> global_counter;

    CStore() {
        storage_layer = new LockfreeList();
        index_layer = new IndexLayer();

    }

    void rebuild() {
        index_layer->build(storage_layer->head, storage_layer->tail);
    }

    void insert(int key, int value) {

        if (storage_layer->insert(key, value)) {
            modification_counter++;
        }

    }


};


#endif //CLION_CSTORE_H
