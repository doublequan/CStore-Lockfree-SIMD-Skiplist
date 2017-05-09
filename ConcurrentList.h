#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include "ConcurrentSortedLinkedList.h"


#define set_is_delete(address) ((Node<T>*)((uintptr_t)address | 1))
#define set_confirm_delete(address) ((Node<T>*)((uintptr_t)address | 2))
#define get_is_delete(address) ((int)((uintptr_t)address & 0x00000001))
#define get_confirm_delete(address) ((int)((uintptr_t)address & 0x00000002) == 2 ?1:0)
#define get_node_address(address) ((Node<T>*)((uintptr_t)address & -4))


template<typename T>
class Node {
public:
    T data;
    Node<T> *next;

    Node(T t) : data(t), next(NULL) {}

    void to_string() {
        printf("%d", data);
    }

    // key of node
    T key;
    // whether this storage node is the starting node pointed to from the upper index layer
    bool is_start_node = false;
    // height of index for this node. 0 for no index.
    int height;
};


template<typename T>
class lockfreeList {
public:

    Node<T> *head;
    Node<T> *tail;

    lockfreeList() {
        head = new Node<T>(NULL);
        tail = new Node<T>(NULL);
        head->next = tail;
    }

    ~lockfreeList() {

    }

    void insert(int value) {
        Node<T> *new_node = new Node<T>(value);
        Node<T> *right_node, *left_node;

        while (true) {
            right_node = search_from(head, value, left_node);
//            if ((right_node != tail) && (right_node->data == value))
//                return; //false;
            new_node->next = right_node;
            if (__sync_bool_compare_and_swap(&left_node->next, right_node, new_node))
                return;// true;
        }
    }

    /**
     * Search value in the list from start_node (exclusive).
     * return the right node value and put left node point in parameter left_node
     * @param value
     * @param left_node
     * @return
     */
    Node<T> *search_from(Node<T> *start_node, int value, Node<T> *&left_node) {
        Node<T> *left_node_next, *right_node;

        SEARCH_AGAIN:
        do {
            Node<T> *t = start_node;
            Node<T> *t_next = start_node->next;
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
        Node<T> *right_node = NULL, *right_node_next = NULL, *left_node = NULL;
        while (true) {
            right_node = search_from(head, value, left_node);
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
        if (!__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next)) {
            right_node = search_from(head, right_node->data, left_node);
        }
        return;// true;
    }

    bool find(int value) {
        Node<T> *right_node, *left_node;
        right_node = search_from(head, value, left_node);
        if ((right_node == tail) || (right_node->data != value)) return false;
        else return true;
    }
    //string printlist();

    /**
     * Non-thread-safe
     */
    int to_string() {
        Node<T> *cur = head;
        int count = 0;
        while (cur->next != tail) {
            cur->next->to_string();
            printf("\n");
            cur = cur->next;
            count++;
        }
        return count;
    }

};


#endif //CLION_LOCKFREE_LIST_H

