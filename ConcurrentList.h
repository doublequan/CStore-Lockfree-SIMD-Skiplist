#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>

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


class Node {
public:
    int data;
    Node *next;

    Node(int t) : data(t), next(NULL) {}

    void to_string() {
        printf("%d", data);
    }

    // key of node
    int key;
    // whether this storage node is the starting node pointed to from the upper index layer
    bool is_start_node = false;
    // height of index for this node. 0 for no index.
    int height;
};


class lockfreeList {
public:

    Node *head;
    Node *tail;

    lockfreeList() {
        head = new Node(NULL);
        tail = new Node(NULL);
        head->next = tail;
    }

    ~lockfreeList() {

    }

    void insert(int value) {
        Node *new_node = new Node(value);
        Node *right_node, *left_node;

        while (true) {
            right_node = search_after(head, value, left_node);
            if ((right_node != tail) && (right_node->data == value))
                return; //false;
            new_node->next = right_node;
            if (__sync_bool_compare_and_swap(&left_node->next, right_node, new_node))
                return;// true;
        }
    }

    /**
     * Search value in the list from start_node (exclusive).
     * Find right and left node such that value is in (left_node.data, right_node.data]
     * return the right node and put left node point in parameter left_node
     * @param value
     * @param left_node
     * @return
     */
    Node *search_after(Node *start_node, int value, Node *&left_node) {
        Node *left_node_next, *right_node;

        SEARCH_AGAIN:
        do {
            Node *t = start_node;
            Node *t_next = start_node->next;
            /* Find left and right node */
            do {
                if (!get_is_delete(t_next)) {
                    (left_node) = t;
                    left_node_next = t_next;
                }
                t = get_node_address(t_next);

                if (t == tail) break;
                t_next = t->next;

            } while (get_is_delete(t_next) || t->data < value);
            right_node = t;

            /* Check nodes are adjacent */
            if (left_node_next == right_node) {
                if ((right_node != tail) && get_is_delete(right_node->next)) {
                    goto SEARCH_AGAIN;
                } else {
                    return right_node;
                }
            }
            if (__sync_bool_compare_and_swap(&((left_node)->next), left_node_next, right_node)) { // RIGHT HERE
                if ((right_node != tail) && get_is_delete(right_node->next)) {
                    goto SEARCH_AGAIN;
                } else {
                    return right_node;
                }
            }

        } while (true);
    }

    void remove(int value) {
        Node *right_node = NULL, *right_node_next = NULL, *left_node = NULL;
        while (true) {
            right_node = search_after(head, value, left_node);
            if ((right_node == tail) || (right_node->data != value)) {
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
//            right_node = search_after(head, right_node->data, left_node);
//        }
        return;// true;
    }

    bool find(int value) {
        Node *right_node, *left_node;
        right_node = search_after(head, value, left_node);
        if ((right_node == tail) || (right_node->data != value)) return false;
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

