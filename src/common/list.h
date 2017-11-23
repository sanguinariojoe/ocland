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

/** @file list.h
 * @brief Something similar to vector in C++
 *
 * @see List
 */

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/** The list
 *
 * Lists should be ever initialized by alloc_list(), and destroyed by
 * free_list().
 * To fill the list you can use list_push_back(), list_push_front() and
 * list_insert() functions.
 * To traverse the list you can either get the list head calling list_head(),
 * subsequently calling list_next() until NULL is returned, or you can
 * optionally use list_at(), which is significantly less efficient. Along this
 * line, these methods are not returning the value, but a Node (some sort of
 * iterator). The node value can be extracted using list_value()
 * @note Lists can only store object refs, i.e. void*
 */
typedef struct __List List;

/** A list node
 *
 * Nodes are useful to iterate/traverse a list, by means of list_head() to get
 * the first node of the list, and list_next() to subsequently get the rest of
 * them. On top of each node you can get the element value calling list_value()
 */
typedef struct __Node Node;

/** Instantiates a new List
 *
 * Remember calling list_free() to release the allocated memory.
 * If the memory allocation eventually fails, an assertion error will be raised.
 *
 * @return The new list
 */
List* list_alloc();

/** Release the memory allocated by list_alloc()
 *
 * @param l The list to be released
 * @warning This is also erasing the nodes, so don't release the list if you are
 * calling list_next() after that
 */
void list_free(List *l);

/** Length of a list
 *
 * @param l The list
 * @return The number of elements allocated in the list
 */
size_t list_len(List *l);

/** Add an element at the head of a list
 *
 * The head of the list is the first node
 * 
 * @param l The list where the element should be inserted
 * @param v The element to be inserted
 */
void list_push_front(List *l, void* v);

/** Add an element at the tail of a list
 *
 * The tail of the list is the last node
 * 
 * @param l The list where the element should be inserted
 * @param v The element to be inserted
 */
void list_push_back(List *l, void* v);

/** Add an element at a specific position of the list
 *
 * The function is raising an assertion error if the position is out of
 * bounds, i.e. #p bigger than list_len()
 *
 * @param l The list where the element should be inserted
 * @param v The element to be inserted
 * @param p Position of the new element
 */
void list_insert(List *l, void* v, size_t p);

/** Remove and return the head element of a list
 *
 * The head of the list is the first node.
 * The function is raising an assertion error if the list is empty
 * 
 * @param l The list from which the element should be removed
 */
void* list_pop_front(List *l);

/** Remove and return the tail element of a list
 *
 * The tail of the list is the last node.
 * The function is raising an assertion error if the list is empty
 * 
 * @param l The list from which the element should be removed
 */
void* list_pop_back(List *l);

/** Remove and return an element of the list at a specific position
 *
 * The function is raising an assertion error if the position is out of
 * bounds, i.e. #p bigger or equal than list_len()
 *
 * @param l The list where the element should be inserted
 * @param p Position of the new element
 */
void* list_erase(List *l, size_t p);

/** Get the first node of the list
 *
 * The value of the node can be accessed by list_value()
 * 
 * @param l The list
 * @return The head element of the list
 */
Node* list_head(List *l);

/** Get the last node of the list
 *
 * The value of the node can be accessed by list_value()
 * 
 * @param l The list
 * @return The tail element of the list
 */
Node* list_tail(List *l);

/** Get a specific node of the list, located at the position #p
 *
 * The function is raising an assertion error if the position is out of
 * bounds, i.e. #p bigger or equal than list_len()
 * 
 * @param l The list
 * @param p Position of the new element
 * @return The tail element of the list
 */
Node* list_at(List *l, size_t p);

/** Get the next node of the list
 *
 * If it is the last node of the list, then a NULL node will be returned, such
 * that you can traverse the whole list with a loop while the next node is not
 * NULL
 *
 * @param n The node
 * @return The next node to #n, NULL if #n is the last element of the list
 */
Node* list_next(Node* n);

/** Get the previous node of the list
 *
 * If it is the first node of the list, then a NULL node will be returned, such
 * that you can traverse the whole list with a loop while the previous node is
 * not NULL
 *
 * @param n The node
 * @return The previous node to #n, NULL if #n is the last element of the list
 */
Node* list_previous(Node* n);

/** Get a node value
 *
 * If the Node is NULL, an assertion error is raised
 * 
 * @param n The node
 * @return The node value
 */
Node* list_value(Node *n);

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */

