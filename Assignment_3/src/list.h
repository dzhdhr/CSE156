//
// Created by Eric on 10/15/2019.
//
#ifndef LIST_LIST_H
#define LIST_LIST_H
#include <stdio.h>

typedef struct ListObj* List;
typedef struct header* package_header;
List newList(void);


void freeList(List * pL);
int length(List L);
int getindex(List L);
void* front(List L);
void* back(List L);
void* get (List L);



void clear(List L);
void moveFront(List L);

void moveBack(List L);
void movePrev(List L);
void moveNext(List L);

void prepend(List L, void* data);
void append(List, void* data);

void insertBefore(List L,void* data);
void insertAfter(List L, void* data);

void deleteFront(List L); // Delete the front element. Pre: length()>0
void deleteBack(List L); // Delete the back element. Pre: length()>0
void delete(List L);
//void printList(FILE* out, List L);

void insertByOrder(List L,struct header* data);
int next_seq(List L,int base);
// of L is unchanged.
#endif //LIST_LIST_H




