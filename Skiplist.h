//
// Created by kenneth on 4/18/17.
//

#ifndef CSTORE_SKIPLIST_H
#define CSTORE_SKIPLIST_H

#include <string>
#include <vector>
#include <stdint.h>
#include <xmmintrin.h>
#include <unordered_map>
#include "compare_ispc.h"
#include "build_index_node_ispc.h"
#include "concurrentLinkedList.h"

#define MAX_HEIGHT 20
#define VECTOR_SIZE 32


class IndexNode{
private:
    std::string indexes[];
public:
    IndexNode* next;
    IndexNode* prev;
    std::unordered_map<std::string, void*> routing_table;
    IndexNode(){
        indexes = new std::string[VECTOR_SIZE];
        next = NULL;
        prev = NULL;
        routing_table = std::unordered_map<std::string, void*>();
    }

    // give an int(representing the compare result), return the pointer to
    // next node to find(could be index node or storage node)
    void* routeToNextNode(std::string key){
        std::string result = std::string(VECTOR_SIZE, '0');
        index_compare_ispc( indexes, key, VECTOR_SIZE, result);
        return routing_table[result];
    }

};

class IndexLayer{
private:
    IndexNode starts[];
    IndexNode ends[];
public:
    IndexLayer(){
        starts = new IndexNode[MAX_HEIGHT];
        ends = new IndexNode[MAX_HEIGHT];
        for(int i = 0; i < MAX_HEIGHT; i++){
            starts[i].next = &ends[i];
        }
    }

    // give a key, return the starting storage node pointer
    Node* find(std::string key){
        int current_level = MAX_HEIGHT -1;
        IndexNode* next_node , *last_node;
//        IndexNode* last_node = &starts[MAX_HEIGHT-1];
        for( ; current_level >= 0; current_level--){
            if(starts[current_level].next != &ends[current_level]){
                last_node = &starts[current_level];
                next_node = last_node->next;
                break;
            }
        }
        // if no index layer is found, return null
        if(current_level < 0){
            return NULL;
        }
        while(current_level >= 0){
            last_node = next_node;
            next_node = (IndexNode*)(next_node->routeToNextNode(key));
        }
        if(next_node == NULL){
            // storage node has been deleted, then start from the last possible index node
            if(last_node->prev == &starts[0]){
                return NULL;
            }else{
                return (Node*)last_node->prev->routeToNextNode(key);
            }
        }
        return (Node*) next_node;

    }
    // give start node of storage layer, rebuild the index layer
    // todo use ISPC to help rebuild each slice of index layer
    void rebuild(Node* start){
        // store current index node of each level
        IndexNode* leveled_nodes[MAX_HEIGHT];
        // store number of indexes per current index node
//        int leveled_nodes_count[MAX_HEIGHT];
        // store the number of indexes exists in each index node
        int level_index_count[MAX_HEIGHT];
        // routing key is in the format of "0000...1111", each time insert an index, replace the corresponding char to 0
        std::string routing_keys[MAX_HEIGHT];
        std::string initial_key = std::string(VECTOR_SIZE, '1');
        for(int i = 0; i < MAX_HEIGHT; i++){
            leveled_nodes[i] = &starts[i];
            level_index_count[i] = VECTOR_SIZE-1;
        }
        Node* cur = start;
        do{
            if(cur->height > 0) {
                build_index_node(leveled_nodes, level_index_count, VECTOR_SIZE, MAX_HEIGHT, cur->height, cur->key,
                                 routing_keys, initial_key);
                set_node_pointer(leveled_nodes, routing_keys, level_index_count, MAX_HEIGHT, cur->height);
                leveled_nodes[0]->routing_table[routing_keys[0]] = cur;
                if(level_index_count[0] == 1){
                    cur->is_start_node = true;
                }
            }
        }while((cur = cur->next)!=NULL);
        for(int i = 0; i < MAX_HEIGHT; i++){
            leveled_nodes[i]->next = &ends[i];
        }
    }
};
#endif //CSTORE_SKIPLIST_H

