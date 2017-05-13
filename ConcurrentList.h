#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <time.h>
#include <random>
#include "constants.h"
#include "Node.h"

#define INDEX_DEBUG

#ifdef INDEX_DEBUG

#include "Skiplist.h"

#endif
/**
 * is_delete flag utilize the last digit of node address,
 * it will be set through set_is_delete,
 * if it is set to 1, means this node is marked to be logically deleted
 * All logic operations should skip such a logically deleted node
 * Later GC will deal with those logically deleted node.
 * Once a node is marked as logically deleted, it will never be back
 */

/**
 * confirm delete flag is used in index_layer rebuild
 */



void *background_job(void *ptr);


class LockfreeList {
public:

    Node *head;
    Node *tail;

    bool index_ready;


#ifdef INDEX_DEBUG

    IndexLayer *index_layer;
#endif

    std::atomic<unsigned int> modification_counter;
    std::atomic<unsigned int> global_counter;

    pthread_t background_thread;


    LockfreeList() {
        global_counter = 0;
        modification_counter = 0;

        head = new Node(NULL, NULL, randomLevel());
        tail = new Node(NULL, NULL, randomLevel());
        head->next = tail;
        srand(time(NULL));

        index_ready = false;

        load_data();

#ifdef INDEX_DEBUG
        index_layer = new IndexLayer();
        index_layer->build(head, tail);
        index_ready = true;
#endif
        pthread_create(&background_thread, NULL, background_job, (void *) this);
    }

    inline unsigned intRand(const unsigned &min, const unsigned &max) {
        static thread_local std::mt19937 generator(time(0));
        std::uniform_int_distribution<unsigned> distribution(min, max);
        return distribution(generator);
    }

    int randomLevel() {
        int level = 0;
        for (unsigned number = intRand(0, UINT32_MAX); (number & 1) == 1 && level < MAX_HEIGHT; number >>= 1) {
            level++;
        }
        return level;
    }

    ~LockfreeList() {
    }


    Node *search_by_index(int key, Node *&left_node) {
        if (!index_ready) {
            return search_after(head, key, left_node);
        }

#ifdef INDEX_DEBUG

        global_counter++;

        IndexLayer *indexLayer = index_layer;

        indexLayer->ongoing_query_counter++;

        Node *n = indexLayer->find(key - 1);

        indexLayer->ongoing_query_counter--;

        global_counter--;

        return search_after(n, key, left_node);

#else
        return search_after(head, key, left_node);
#endif
    }


    void insert(int key, int value) {
        Node *new_node = new Node(key, value, randomLevel());
        Node *right_node, *left_node;

        while (true) {
            right_node = search_by_index(key, left_node);
            if ((get_node_address(right_node) != tail) && !get_is_delete(get_node_address(right_node)->next)
                && (get_node_address(right_node)->key == key))
                return; //false;
            get_node_address(new_node)->next = get_node_address(right_node);
            new_node = copy_mask(get_node_address(left_node)->next, new_node);
            if (__sync_bool_compare_and_swap(&get_node_address(left_node)->next, right_node, new_node)) {

                modification_counter++;

                return;// true;
            }
        }
    }

    /**
     * Search value in the list from start_node (exclusive).
     * Find right and left node such that value is in (left_node.data, right_node.data]
     * return the right node and put left node point in parameter left_node
     * @param key
     * @param left_node
     * @return
     */
    Node *search_after(Node *start_node, int key, Node *&left_node) {
        Node *left_node_next, *right_node;

        SEARCH_AGAIN:
        do {

            Node *t = start_node;
            Node *t_next = start_node->next;

            left_node = start_node;
            left_node_next = start_node->next;
            /* Find left and right node */
            do {
                if (!get_confirm_delete(t_next)) {
                    (left_node) = t;
                    left_node_next = t_next;
                }
                t = t_next;

                if (get_node_address(t) == tail) break;
                t_next = get_node_address(t)->next;

            } while (get_confirm_delete(t_next) || get_node_address(t)->key < key);

            right_node = t;

            /* Check nodes are adjacent */
            if (get_node_address(left_node_next) == get_node_address(right_node)) {
                if ((get_node_address(right_node) != tail) && get_confirm_delete(get_node_address(right_node)->next)) {
                    goto SEARCH_AGAIN;
                } else {
                    return right_node;
                }
            }

            Node *masked_right_node = copy_mask((left_node)->next, right_node);

            if (__sync_bool_compare_and_swap(&((get_node_address(left_node))->next), left_node_next,
                                             masked_right_node)) { // RIGHT HERE
                if ((get_node_address(right_node) != tail) && get_confirm_delete(get_node_address(right_node)->next)) {
                    goto SEARCH_AGAIN;
                } else {
                    return right_node;
                }
            }

        } while (true);
    }

    void remove(int key) {
        Node *right_node = NULL, *right_node_next = NULL, *left_node = NULL;
        while (true) {
            right_node = get_node_address(search_by_index(key, left_node));
            if ((right_node == tail) || (right_node->key != key)) {
                printf("wrong return\n");
                return;
            }// false;
            right_node_next = right_node->next;
            if (!get_is_delete(right_node_next)) {


                if (__sync_bool_compare_and_swap(&(right_node->next), right_node_next,
                                                 set_is_delete(right_node_next))) {


                    modification_counter++;

                    break;
                }
            } else {
                return;
            }
        }
        return;// true;
    }

    int find(int key) {
        Node *right_node, *left_node;
        right_node = search_by_index(key, left_node);
        if ((right_node == tail) || (right_node->key != key)) {
            return NULL;
        } else {
            return right_node->value;
        }
    }

    /**
     * Non-thread-safe
     */
    int to_string() {
        Node *cur = head->next;
        int count = 0;
        while (get_node_address(cur) != tail) {
            if (!get_is_delete(cur->next)) {
                count++;
            } else {
                if (get_confirm_delete(cur->next)) {
                    printf("C:");
                }
                printf("D:");
            }
            cur->to_string();
            printf("\n");
            cur = get_node_address(cur->next);
        }
        return count;
    }

    /**
     * Non-thread-safe
     */
    void load_data() {
#ifdef INDEX_DEBUG
        for (int i = INIT_SET_SIZE; i >= 0; i--) {
            insert(i, i);
        }
        modification_counter = 0;
#endif

    }
};


void *background_job(void *ptr) {

#ifdef INDEX_DEBUG

    LockfreeList *lockfreeList = (LockfreeList *) ptr;

    while (true) {
        unsigned counter = lockfreeList->modification_counter;

        if (counter >= REBUILD_THRESHOLD) {
            IndexLayer *old_index_layer = lockfreeList->index_layer;


            IndexLayer *new_index_layer = new IndexLayer();
            new_index_layer->build(lockfreeList->head, lockfreeList->tail);

            lockfreeList->index_layer = new_index_layer;

            while (lockfreeList->global_counter
                   != old_index_layer->ongoing_query_counter
                      + new_index_layer->ongoing_query_counter) {
                usleep(1000);
            }

            while (old_index_layer->ongoing_query_counter != 0) {
                usleep(1000);
            }


            lockfreeList->modification_counter = 0;

        }

        usleep(1000);
    }

#endif
}

#endif //CLION_LOCKFREE_LIST_H

