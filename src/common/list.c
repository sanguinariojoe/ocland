/*
 * Copyright (C) 2017 Jose Luis Cercos-Pita <jlcercos@gmail.com>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

/** @file list.c
 * @brief Something similar to vector in C++
 *
 * @note Lists can only store object refs, i.e. void*
 */

#include <stdlib.h>
#include <assert.h>
#include "list.h"


/** Data structure for a Node
 *
 * @see Node
 */
struct __Node {
    Node *next;
    Node *prev;
    void* val;
};

/** Data structure for a List
 *
 * @see List
 */
struct __List {
    Node *head;
    Node *tail;
    size_t len;
};

List* list_alloc()
{
    List *l = malloc(sizeof(List));
    assert(l != NULL);
    l->head = l->tail = NULL;
    l->len = 0;
    return l;
}

void list_free(List *l)
{
    size_t i;
    for(i = 0; i < l->len; ++i){
        list_pop_back(l);
    }
    free(l);
}

size_t list_len(List *l)
{
    return l->len;
}

void list_push_front(List *l, void* v)
{
    Node *n = malloc(sizeof(Node));
    assert(n != NULL);
    n->val = v;
    n->next = l->head;
    n->prev = NULL;
    if (l->tail == NULL) {
        l->head = l->tail = n;
    } else {
        l->head->prev = n;
        l->head = n;
    }
    l->len++;
}

void list_push_back(List *l, void* v)
{
    Node *n = malloc(sizeof(Node));
    assert(n != NULL);
    n->val = v;
    n->prev = l->tail;
    n->next = NULL;
    if (l->head == NULL) {
        l->head = l->tail = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    l->len++;
}

void list_insert(List *l, void* v, size_t p)
{
    assert(p <= l->len);
    if(p == 0){
        list_push_front(l, v);
        return;
    }
    if(p == l->len){
        list_push_back(l, v);
        return;
    }
    Node *n = malloc(sizeof(Node));
    assert(n != NULL);
    Node *next = list_at(l, p);
    Node *prev = list_previous(next);
    n->val = v;
    n->next = next;
    n->prev = prev;
    prev->next = n;
    next->prev = n;
    l->len++;
}

void* list_pop_front(List *l)
{
    Node *n = l->head;
    assert(n != NULL);
    void* v = n->val;
    l->head = n->next;
    l->len--;
    if(l->len == 0){
        l->tail = NULL;
    }
    free(n);
    return v;
}

void* list_pop_back(List *l)
{
    Node *n = l->tail;
    assert(n != NULL);
    void* v = n->val;
    l->tail = n->prev;
    l->len--;
    if(l->len == 0){
        l->head = NULL;
    }
    free(n);
    return v;
}

void* list_erase(List *l, size_t p)
{
    assert(p < l->len);
    if(p == 0){
        return list_pop_front(l);
    }
    if(p == l->len - 1){
        return list_pop_back(l);
    }
    Node *n = list_at(l, p);
    assert(n != NULL);
    void* v = n->val;
    n->next->prev = n->prev;
    n->prev->next = n->next;
    free(n);
    return v;
}

Node* list_head(List *l)
{
    return l->head;
}

Node* list_tail(List *l)
{
    return l->tail;
}

Node* list_at(List *l, size_t p)
{
    assert(p < l->len);
    if(p == 0){
        return list_head(l);
    }
    if(p == l->len - 1){
        return list_tail(l);
    }
    size_t i;
    Node *n;
    if(l->len - p < p){
        // The element is closer to the end of the list
        n = l->tail;
        for(i = 1; i < l->len - p; ++i){
            n = n->prev;
        }        
    }
    else{
        n = l->head;
        for(i = 0; i < p; ++i){
            n = n->next;
        }
    }
    return n;
}

Node* list_next(Node* n)
{
    assert(n != NULL);
    return n->next;
}

Node* list_previous(Node* n)
{
    assert(n != NULL);
    return n->prev;    
}

Node* list_value(Node *n)
{
    assert(n != NULL);
    return n->val;    
}