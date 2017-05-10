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
#include <iostream>
#include "compare_ispc.h"
//#include "build_index_node_ispc.h"
//#include "ConcurrentList.h"
#include "Node.h"
#include "constants.h"

// #define DEBUG
class IndexNode {
public:
    int indexes[VECTOR_SIZE];
    IndexNode *next;
    IndexNode *prev;
//    std::unordered_map<std::string, void*> routing_table;
    void *routing_array[VECTOR_SIZE];
    int index_size = VECTOR_SIZE;


    IndexNode() {
        next = NULL;
        prev = NULL;
    }

    // give an int(representing the compare result), return the pointer to
    // next node to find(could be index node or storage node)
    void *routeToNextNode(int key) {
#ifdef DEBUG
        printf("begin routing for key %d. current indexes:\n", key);
        print_indexes();
        printf("\n");
#endif
//        std::string result = std::string(VECTOR_SIZE, '1');
//        int8_t result_char[VECTOR_SIZE];
//        index_compare_ispc( indexes, key, index_size, result);
        // std::string result = compare(key);
        int8_t result[VECTOR_SIZE + 1];
        ispc::index_compare_ispc(indexes, key, VECTOR_SIZE, index_size, result);
#ifdef DEBUG
        printf("compare result %s, going to node %p\n", result.c_str(), routing_table[result]);
#endif
        if (result[index_size - 1] == '1') {
            for (int i = index_size - 2; i >= 0; i--) {
                if (result[i] == '0') {
                    return routing_array[i + 1];
                }
            }
            return routing_array[0];
        }
        if (index_size < VECTOR_SIZE) {
            return routing_array[index_size];
        }
        return NULL;
    }

    uint64_t compare(int key) {
        uint8_t result[VECTOR_SIZE + 1];
        for (int i = 0; i < VECTOR_SIZE; i++) {
            if (i < index_size && key >= indexes[i]) {
                result[i] = 0;
            } else {
                result[i] = 1;
            }
        }
        return *((uint64_t *) result);
    }

    // std::string compare(int key){
    //     char result[VECTOR_SIZE+1];
    //     for(int i = 0; i < VECTOR_SIZE; i++){
    //         if(i < index_size && key >= indexes[i]){
    //             result[i] = '0';
    //         }else{
    //             result[i] = '1';
    //         }
    //     }
    //     result[VECTOR_SIZE] = 0;
    //     std::string str_result = std::string(result);
    //     return str_result;
    // }

    void print_indexes() {
        for (int i = 0; i < index_size; i++) {
            printf("%d ", indexes[i]);
        }
    }

};

class IndexLayer {
private:
    IndexNode starts[MAX_HEIGHT];
    IndexNode ends[MAX_HEIGHT];
public:
    std::atomic<unsigned int> ongoing_query_counter;

    IndexLayer() {
        ongoing_query_counter = 0;

        for (int i = 0; i < MAX_HEIGHT; i++) {
            starts[i].next = &ends[i];
        }
    }

    void print_index_layers() {
        for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
            IndexNode *cur = &starts[i];
            while ((cur = cur->next) != &ends[i]) {
                cur->print_indexes();
                printf("\t");
            }
            printf("end\n");
        }
    }

    void print_layer_pointers() {
        for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
            IndexNode *cur = &starts[i];
            printf("%p\t", cur);
            while ((cur = cur->next) != &ends[i]) {
                printf("%p\t", cur);
            }
            printf("end\n");
        }
    }

    // give a key, return the starting storage node pointer
    Node *find(int key) {

        int current_level = MAX_HEIGHT - 1;

#ifdef DEBUG
        printf("find key %d starts, current level: %d\n", key, current_level);
#endif
        IndexNode *next_node, *last_node;
//        IndexNode* last_node = &starts[MAX_HEIGHT-1];
        for (; current_level >= 0; current_level--) {
            if (starts[current_level].next != &ends[current_level]) {
                last_node = &starts[current_level];
                next_node = last_node->next;
#ifdef DEBUG
                printf("level %d has index node, go to the first node on this level\n", current_level);
#endif
                break;
            }
#ifdef DEBUG
            printf("level %d has no index node, go down one level\n", current_level);
#endif
        }
        // if no index layer is found, return null
        if (current_level < 0) {
            return NULL;
        }
        while (current_level >= 0) {
            last_node = next_node;
            next_node = (IndexNode *) (next_node->routeToNextNode(key));
            if (next_node == NULL) {
#ifdef DEBUG
                printf("down route not found, go to the next index node on level %d\n", current_level);
#endif
                next_node = last_node->next;
                if (next_node == &ends[current_level]) {
#ifdef DEBUG
                    printf("reach end of index layer\n");
#endif
                    int tmp_index = last_node->indexes[last_node->index_size - 1];
                    next_node = (IndexNode *) (last_node->routeToNextNode(tmp_index));
                    current_level--;
                }
            } else {
                current_level--;
            }
        }
        return (Node *) next_node;
    }

    // give start node of storage layer, rebuild the index layer
    // todo use ISPC to help rebuild each slice of index layer
    void build(Node *head, Node *tail) {
        // store current index node of each level
        IndexNode *leveled_nodes[MAX_HEIGHT];
        // store the number of indexes exists in each index node
        int level_index_count[MAX_HEIGHT];
        // routing key is in the format of "0000...1111", each time insert an index, replace the corresponding char to 0
//        std::string routing_keys[MAX_HEIGHT];
//        std::string initial_key = std::string(VECTOR_SIZE, '1');
//        std::string end_key = std::string(VECTOR_SIZE, '0');
        for (int i = 0; i < MAX_HEIGHT; i++) {
            leveled_nodes[i] = &starts[i];
            level_index_count[i] = VECTOR_SIZE - 1;
//            routing_keys[i] = initial_key;
        }
        Node *cur = head;
        while ((cur = get_node_address(cur->next)) != tail) {
#ifdef DEBUG
            printf("current node key %d index height %d\n", cur->key, cur->height);
#endif
            if (get_is_delete(cur->next)) {
                cur->next = set_confirm_delete(cur->next);
            }
            else if (cur->height > 0) {
//                build_index_node(leveled_nodes, level_index_count, VECTOR_SIZE, MAX_HEIGHT, cur->height, cur->key,
//                                 routing_keys, initial_key);
//                set_node_pointer(leveled_nodes, routing_keys, level_index_count, MAX_HEIGHT, cur->height);
                for (int i = 0; i < cur->height; i++) {
                    IndexNode *node = leveled_nodes[i];
                    node->indexes[level_index_count[i]++] = cur->key;
#ifdef DEBUG
                    printf("push key %d into index node %p on level %d, current node index size %d\n", cur->key, node,
                           i, level_index_count[i]+1);
#endif
//                    routing_keys[i][level_index_count[i]] = '0';
                    if (level_index_count[i] != VECTOR_SIZE) {
                        if (i != 0) {
//                        node->routing_table[routing_keys[i]] = leveled_nodes[i-1];
                            node->routing_array[level_index_count[i]] = leveled_nodes[i - 1];
#ifdef DEBUG
                            printf("node %p route to index node %p on compare result %s\n", node, leveled_nodes[i-1], routing_keys[i].c_str());
#endif
                        } else {
//                        leveled_nodes[0]->routing_table[routing_keys[0]] = cur;
                            node->routing_array[level_index_count[i]] = cur;
#ifdef DEBUG
                            printf("node %p route to storage node %p on compare result %s\n", node, cur, routing_keys[0].c_str());
#endif
                        }
                    }
                    if (level_index_count[i] == VECTOR_SIZE) {
#ifdef DEBUG
                        printf("node %p index vector full\n", node);
#endif
                        IndexNode *new_node = new IndexNode();
                        new_node->indexes[0] = cur->key;
                        leveled_nodes[i] = new_node;
                        level_index_count[i] = 1;
//                        routing_keys[i] = initial_key;
//                        node->routing_table[end_key] = NULL;
                        node->next = new_node;
//                        routing_keys[i][0] = '0';
                        if (i != 0) {
//                            new_node->routing_table[routing_keys[i]] = leveled_nodes[i-1];
                            new_node->routing_array[1] = leveled_nodes[i - 1];
                        } else {
//                            new_node->routing_table[routing_keys[i]] = cur;
                            new_node->routing_array[1] = cur;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < MAX_HEIGHT; i++) {
            leveled_nodes[i]->index_size = level_index_count[i];
            leveled_nodes[i]->next = &ends[i];
//            leveled_nodes[i]->routing_table[end_key] = NULL;
            if (i == 0) {
//                starts[i].next->routing_table[initial_key] = head->next;
                starts[i].next->routing_array[0] = head->next;
            } else {
//                starts[i].next->routing_table[initial_key] = starts[i-1].next;
                starts[i].next->routing_array[0] = starts[i - 1].next;
            }
        }
    }
};

#endif //CSTORE_SKIPLIST_H

