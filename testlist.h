#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include "ConcurrentSortedLinkedList.h"


#define setMark(address) ((Node<T>*)((uintptr_t)address | 2))
#define setFlag(address) ((Node<T>*)((uintptr_t)address | 1))
#define getMark(address) ((int)((uintptr_t)address & 0x00000002) == 2 ?1:0)
#define getFlag(address) ((int)((uintptr_t)address & 0x00000001))
#define getNodeAddress(address) ((Node<T>*)((uintptr_t)address & -4))


template<typename T>
class Node {
public:
    T data;
    Node<T> *next;
    Node<T> *prev;

    bool is_deleted;;

    Node(T t) : data(t), next(NULL), is_deleted(false) {}

    void to_string() {
        printf("%d", data);
        if (is_deleted) printf(":d");
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
            right_node = search(value, left_node);
//            if ((right_node != tail) && (right_node->data == value))
//                return; //false;
            new_node->next = right_node;
            if (__sync_bool_compare_and_swap(&left_node->next, right_node, new_node))
                return;// true;
        }

    }

    Node<T> *search(int value, Node<T> *&left_node) {
        Node<T> *left_node_next, *right_node;

        SEARCH_AGAIN:
        do {
            Node<T> *t = head;
            Node<T> *t_next = head->next;
            /* Find left and right node */
            do {
                if (!getMark(t_next)) {
                    (left_node) = t;
                    left_node_next = t_next;
                }
                t = getNodeAddress(t_next);

                if (t == tail) break;
                t_next = t->next;

            } while (getMark(t_next) || t->data < value);
            right_node = t;

            /* Check nodes are adjacent */
            if (left_node_next == right_node) {
                if ((right_node != tail) && getMark(right_node->next)) {
                    goto SEARCH_AGAIN;
                } else {
                    return right_node;
                }
            }
            if (__sync_bool_compare_and_swap(&((left_node)->next), left_node_next, right_node)) { // RIGHT HERE
                if ((right_node != tail) && getMark(right_node->next)) {
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
            right_node = search(value, left_node);
            if ((right_node == tail) || (right_node->data != value)) {
                return;
            }// false;
            right_node_next = right_node->next;
            if (!getMark(right_node_next)) {
                if (__sync_bool_compare_and_swap(&(right_node->next), right_node_next,
                                                 setMark(right_node_next)))
                    break;
            }
        }
        if (!__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next)) {
            right_node = search(right_node->data, left_node);
        }
        return;// true;
    }

    bool find(int value) {
        Node<T> *right_node, *left_node;
        right_node = search(value, left_node);
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

