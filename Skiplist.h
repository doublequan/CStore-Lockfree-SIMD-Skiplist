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



//#include "build_index_node_ispc.h"
#include "ConcurrentList.h"
#include "constants.h"
#include "Node.h"


#define ISPC
#ifdef ISPC
#include "compare_ispc.h"

#endif

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
#ifdef ISPC

        int count = ispc::index_compare_ispc_sum(indexes, key, VECTOR_SIZE, index_size);
        if(count == 0){
            return NULL;
        }
        return routing_array[VECTOR_SIZE - count];
#else
        for (int i = 0; i < index_size; i++) {
            if (key < indexes[i]) {
                return routing_array[i];
            }
        }
        if (index_size < VECTOR_SIZE) {
            return routing_array[index_size];
        }
        return NULL;
#endif
#ifdef DEBUG
        printf("compare result %s, going to node %p\n", result.c_str(), routing_table[result]);
#endif
    }

    uint8_t *compare(int key) {
        uint8_t *result = new uint8_t[VECTOR_SIZE];
        for (int i = 0; i < VECTOR_SIZE; i++) {
            if (i < index_size && key >= indexes[i]) {
                result[i] = 0;
            } else {
                result[i] = 1;
            }
        }
        return result;
    }

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
    Node *head;
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
        // if no index layer is found, return head
        if (current_level < 0) {
            return head;
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
        this->head = head;
        // store current index node of each level
        IndexNode *leveled_nodes[MAX_HEIGHT];
        // store the number of indexes exists in each index node
        int level_index_count[MAX_HEIGHT];
        // routing key is in the format of "0000...1111", each time insert an index, replace the corresponding char to 0
        for (int i = 0; i < MAX_HEIGHT; i++) {
            leveled_nodes[i] = &starts[i];
            level_index_count[i] = VECTOR_SIZE - 1;
        }
        Node *cur = head;
        Node *last = NULL;
        while ((cur = get_node_address(cur->next)) != tail) {

#ifdef DEBUG
            printf("current node key %d index height %d\n", cur->key, cur->height);
#endif
            if (last != NULL) {

                while (true) {
                    Node *old_last_next = last->next;
                    if (__sync_bool_compare_and_swap(&(last->next), old_last_next,
                                                     set_is_delete(old_last_next))) {
                        break;
                    }
                }
                last = NULL;
            }
            if (get_is_delete(cur->next)) {
                last = cur;
            } else if (cur->height > 0) {
                for (int i = 0; i < cur->height; i++) {
                    IndexNode *node = leveled_nodes[i];
                    node->indexes[level_index_count[i]++] = cur->key;
#ifdef DEBUG
                    printf("push key %d into index node %p on level %d, current node index size %d\n", cur->key, node,
                           i, level_index_count[i]+1);
#endif
                    if (level_index_count[i] != VECTOR_SIZE) {
                        if (i != 0) {
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
                        node->next = new_node;
                        if (i != 0) {
                            new_node->routing_array[1] = leveled_nodes[i - 1];
                        } else {
                            new_node->routing_array[1] = cur;
                        }
                    }
                }
            }
        }
        if (last != NULL) {

            while (true) {
                Node *old_last_next = last->next;
                if (__sync_bool_compare_and_swap(&(last->next), old_last_next,
                                                 set_is_delete(old_last_next))) {
                    break;
                }
            }
            last = NULL;
        }
        for (int i = 0; i < MAX_HEIGHT; i++) {
            leveled_nodes[i]->index_size = level_index_count[i];
            leveled_nodes[i]->next = &ends[i];
            if (i == 0) {
                starts[i].next->routing_array[0] = head->next;
            } else {
                starts[i].next->routing_array[0] = starts[i - 1].next;
            }
        }
    }
};

#endif //CSTORE_SKIPLIST_H
