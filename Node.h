//
// Created by Quan Quan on 5/10/17.
//

#ifndef CLION_NODE_H
#define CLION_NODE_H

#include <stdlib.h>

#define set_is_delete(address) ((Node *)((uintptr_t)address | 1))
#define set_confirm_delete(address) ((Node *)((uintptr_t)address | 2))
#define get_is_delete(address) ((int)((uintptr_t)address & 0x00000001))
#define get_confirm_delete(address) ((int)((uintptr_t)address & 0x00000002) == 2 ?1:0)
#define get_node_address(address) ((Node *)((uintptr_t)address & -4))

#define copy_mask(src, dest) ((Node *) (((uintptr_t) dest & -4) | ((uintptr_t) src & 3)))

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


#endif //CLION_NODE_H
