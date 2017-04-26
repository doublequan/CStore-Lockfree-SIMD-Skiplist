#ifndef CLION_LOCKFREE_LIST_H
#define CLION_LOCKFREE_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <mutex>

template<typename T>
typedef
struct struct_node {
    struct_node() : data(NULL), next(NULL), is_deleted(false) {};
    T *data;
    struct_node *next;
    bool is_deleted;
} Node;

template<class T>
class ConcurrentLinkedList {
public:
    // an atomic counter indicates the number of modification made to the list
    std::atomic<unsigned int> modifcation_num;
    Node<T> *head;
    std::mutex gc_lock;

    ConcurrentLinkedList() {
        head = (Node<T> *)malloc(sizeof(Node));
    }

    /**
     * Insert node n after node p
     * @param p
     * @param n
     * @return
     */
    void insert(Node *p, Node *n) {
        if (p == NULL || n == NULL) return;
        while (1) {
            n->next = p->next;
            Node *old_next = p->next;
            if (__sync_val_compare_and_swap(&p->next, old_next, n) == old_next)
                modifcation_num++;
                return;
        }
    }

    /**
     * Remove node n
     * Here we mark it as deleted, it will be deleted by the GC later
     * @param n
     */
    void remove(Node *n) {
        if (n == NULL) return;
        n->is_deleted = true;
        modifcation_num++;
    }

    /**
     * The function assume the node sorted by data in ascending order
     * Search from node p, if there is node's data == t, return that node
     * Otherwise return NULL
     * @param p
     * @param t
     * @return
     */
    Node *search_from(Node *p, T t) {
        if (p == NULL) return NULL;

        if (p->data == t && !p->is_deleted) return p;

        Node *cur = p;
        while (cur->next != NULL && cur->next->data <= t) {
            cur = cur->next;
            if (cur->data == t && !cur->is_deleted) return cur;
        }
        return NULL;
    }

    /**
     * traverse the list and delete all marked node
     * There should be only one Garbage Collection function running at the same time
     */
    void garbage_collect() {
        gc_lock.lock();

        Node *cur = head;
        while (cur->next != NULL) {
            if (cur->next->is_deleted) {
                Node *tmp = cur->next;
                cur->next = cur->next->next;
                free(tmp);
            } else {
                cur = cur->next;
            }
        }

        gc_lock.unlock();
    }
};

#endif //CLION_LOCKFREE_LIST_H
