#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <stdlib.h>
#include <time.h>
#include <random>
#include "constants.h"

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
 * TODO: more details need to be added
 */
#define set_is_delete(address) ((Node *)((uintptr_t)address | 1))
#define set_confirm_delete(address) ((Node *)((uintptr_t)address | 2))
#define get_is_delete(address) ((int)((uintptr_t)address & 0x00000001))
#define get_confirm_delete(address) ((int)((uintptr_t)address & 0x00000002) == 2 ?1:0)
#define get_node_address(address) ((Node *)((uintptr_t)address & -4))
#define get_node_address_ignore_last_digit(address) ((Node *)((uintptr_t)address & -2))


class Node {
public:
    int key;
    int value;
    Node *next;

    Node(int k, int v, int h) : key(k), value(v), next(NULL), height(h) {}

    void to_string() {
        printf("%d", key);
    }

    // height of index for this node. 0 for no index.
    int height;
};


class lockfreeList {
public:

    Node *head;
    Node *tail;

    lockfreeList() {
        head = new Node(NULL, NULL, randomLevel() );
        tail = new Node(NULL, NULL, randomLevel() );
        head->next = tail;
        srand (time(NULL));
    }

    inline int intRand(const int & min, const int & max) {
        static thread_local std::mt19937 generator(time(0));
        std::uniform_int_distribution<int> distribution(min,max);
        return distribution(generator);
    }

    int randomLevel () {
        int v = 0;

        while ((((double)intRand(0, 100) / 100)) < 0.5 &&
               v < MAX_HEIGHT) {
            v += 1;
        }
        return v;
    }

    ~lockfreeList() {

    }

    void insert(int key, int value) {
        Node *new_node = new Node(key, value, randomLevel());
        Node *right_node, *left_node;

        while (true) {
            right_node = search_after(head, key, left_node);
            if ((get_node_address(right_node) != tail) && !get_is_delete(get_node_address(right_node)->next)
                && (get_node_address(right_node)->key == key))
                return; //false;
            get_node_address(new_node)->next = get_node_address(right_node);
            if (get_is_delete(get_node_address(left_node)->next)) {
                new_node = set_is_delete(new_node);
            } else {
                new_node = get_node_address(new_node);
            }
            if (__sync_bool_compare_and_swap(&get_node_address(left_node)->next, right_node, new_node))
                return;// true;
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
            /* Find left and right node */
            do {
                if (!get_confirm_delete(t_next)) {
                    (left_node) = t;
                    left_node_next = t_next;
                }
//                t = get_node_address(t_next);
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
            if (__sync_bool_compare_and_swap(&((left_node)->next), left_node_next, right_node)) { // RIGHT HERE
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
            right_node = search_after(head, key, left_node);
            if ((right_node == tail) || (right_node->key != key)) {
                return;
            }// false;
            right_node_next = right_node->next;
            if (!get_is_delete(right_node_next)) {
                if (__sync_bool_compare_and_swap(&(right_node->next), right_node_next,
                                                 set_is_delete(right_node_next)))
                    break;
            }
        }
        // try to physically delete the node
//        if (!__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next)) {
//            right_node = search_after(head, right_node->key, left_node);
//        }
        return;// true;
    }

    bool find(int key) {
        Node *right_node, *left_node;
        right_node = search_after(head, key, left_node);
        if ((right_node == tail) || (right_node->key != key)) return false;
        else return true;
    }
    //string printlist();

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
                printf("D:");
            }
            cur->to_string();
            printf("\n");
            cur = get_node_address(cur->next);
        }
        return count;
    }

};


#endif //CLION_LOCKFREE_LIST_H
