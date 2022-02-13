//
// Created by Eric on 10/15/2019.
//


#include <stdio.h>
#include <stdlib.h>
#include "list.h"
//define node
typedef struct NodeObj{
  void* data;
  struct NodeObj* next;
  struct NodeObj* prev;
}NodeObj;

struct header{
  int size;
  int seq;
  int type;
  char payload[2048];
  int window_size;
  char filename[64];
  int base_seq;
  int count;
};

typedef NodeObj* Node;
// create a node
Node newNode(void* data){
  Node ret = malloc(sizeof(NodeObj));

  ret->data=data;
  ret->prev=NULL;
  ret->next=NULL;
  return ret;
}
//释放所有变量
//free node element
void freeNode(Node *node){
  if(node!=NULL&&*node!=NULL){
    free(*node);
    *node=NULL;
  }
}
//Listd构造函数
//define struct List Object
typedef struct ListObj{
  Node front;
  Node back;
  int length;
  Node cursor;
  int index;
} ListObj;


//Listd构造函数
//create new node
List newList(void){
  List ret = malloc(sizeof(ListObj));

  ret->length=0;
  ret->index=-1;
  ret->front=ret->back=ret->cursor=NULL;
  return ret;
}
//删除所有
//free all the Node
void deleteAll(List L){
  Node n = NULL;
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling deleteAll() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  while( L->length > 0 ){
    n = L->front;
    L->front = L->front->next;
    n->next = NULL;
    n->prev=NULL;
    freeNode(&n);
    L->length--;
  }
}
// free the list and its memory
void freeList(List* pL){ // Passes in List* => ListObj**, passing address of pointer
  if(pL !=NULL && *pL != NULL) {
    while( length(*pL) > 0 ) {
      deleteFront(*pL);
    }
    free(*pL);
    *pL = NULL;
  }
}
//return length of the list
int length(List L){
  if(L==NULL){
    printf("list is empty\n");
    exit(1);
  }
  return L->length;
}
//get the index of cursor if undefined return -1
int getindex(List list){
  if(list==NULL){
    printf("list is empty\n");
    exit(1);
  }
  return list->index;
}
// get the front element
void* front(List L){
  if (L == NULL){
    printf("List Error: calling front() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  else if(length(L)==0){
    printf("List Error: calling front() on empty List reference\n");
    exit(EXIT_FAILURE);
  }
  return L->front->data;
}
//get the back element
void* back(List L){
  if (L == NULL){
    printf("List Error: calling front() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  else if(length(L)==0){
    printf("List Error: calling front() on empty List reference\n");
    exit(EXIT_FAILURE);
  }
  return L->back->data;
}
//get the cursor element
void* get(List L){
  if(L==NULL){
    printf("List Error: calling get() on NULL List\n");
    exit(EXIT_FAILURE);
  }
  if(L->index==-1){
    printf("List Error: calling get() on NULL cursor element\n");
    exit(EXIT_FAILURE);
  }
  return L->cursor->data;
}
void insertByOrder(List L,struct header* data){
  int i = 0;
  if (length(L)==0){
    append(L,data);
    return;
  }
  moveFront(L);
  while (i<length(L)){
    struct header *t= get(L);
    if (data->seq<t->seq){
      insertBefore(L,data);
      return;
    }
    else if (data->seq==t->seq){
      t->base_seq = data->base_seq;
      return;
    }
    moveNext(L);
    i++;
  }
  append(L,data);
}
int next_seq(List L,int base){
  int i = 0;
  if (length(L)==0){
//    printf("empty %d\n",base);
    return base-1;
  }

  moveFront(L);
  struct header* temp = get(L);
  if (base<temp->seq){ //the base one is missing
//    printf("first missing base :%d first: %d\n",temp->base_seq,temp->seq);
    return base-1;
  }
  while (i<length(L)-1) {
    temp = get(L);
   struct header*  next = L->cursor->next->data;
   if (temp->seq+1!= next->seq){
//     printf("find discontent\n");
     return temp->seq;
   }
   i++;
   moveNext(L);
  }
  temp = L->back->data;
//  printf("last\n");
  return temp->seq;
  }


//int equals(List A, List B){
//    if (A == NULL || B == NULL){
//        printf("List Error: calling equals() on NULL List\n");
//        exit(EXIT_FAILURE);
//    }
//    if (length(A) != length(B))
//        return(0);
//
//
//    Node t1 = A->front;
//    Node t2 = B->front;
//    while (t1 != NULL && t2 != NULL){
//
//        if (t1->data != t2->data){
//            return(0);
//        }
//
//        // update otherwise
//        t1 = t1->next;
//        t2 = t2->next;
//    }
//    return(1);
//}
//cheak is the list is empty
int isEmpty(List L){
  if( L==NULL ){
    printf("Queue Error: calling isEmpty() on NULL Queue reference\n");
    exit(EXIT_FAILURE);
  }
  return(L->length==0);
}
void clear(List L){
  if(L==NULL){
    printf("could not call null List\n");
    exit(EXIT_FAILURE);
  }
  while(L->length>0){
    deleteFront(L);
  }
  L->front=L->back=L->cursor=NULL;

  L->length=0;
  L->index=-1;
}
//move to the front of the list
void moveFront(List L){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveFront() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  if(L->length<=0){
    fprintf(stderr,
            "List Error: calling moveFront() on length 0 List reference\n");
    exit(EXIT_FAILURE);
  }
  L->cursor=L->front;
  L->index = 0;
}
void moveBack(List L){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  if(L->length<=0){
    fprintf(stderr,
            "List Error: calling moveBack() on length 0 List reference\n");
    exit(EXIT_FAILURE);
  }
  L->cursor=L->back;
  L->index = L->length-1;
}
//move cursor to the prev element of the list
void movePrev(List L){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  if(L->length<=0){
    fprintf(stderr,
            "List Error: calling moveBack() on length 0 List reference\n");
    exit(EXIT_FAILURE);
  }
  if(L->index!=0&&L->cursor!=NULL){
    L->cursor  = L->cursor->prev;
    L->index = L->index-1;
  }
  else if (L->cursor!=NULL&&L->index==0){
    L->index=-1;
    L->cursor = NULL;
  }
}
//move cursor to the next element of the list
void moveNext(List L){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling movenext() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  if(L->length<=0){
    fprintf(stderr,
            "List Error: calling movenext() on length 0 List reference\n");
    exit(EXIT_FAILURE);
  }
  //如果当前cursor undefined 那就移动到第一个
  if (L->index==-1){
    moveFront(L);
    return;
  }
  //如果当前cursor在最后一个那么cursor改成undefined
  else if(L->index==L->length-1){
    L->index=-1;
    L->cursor=NULL;
    return;
  }
  //如果cursor在其他情况后移
  else {
    L->cursor = L->cursor->next;
    L->index++;
    return;
  }
}
//insert at the back of the list
void prepend(List L, void* data){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  Node node = newNode(data);
  if(L->length==0){
    L->front=L->back=node;
  }
  else{
    node->next = L->front;
    L->front->prev = node;
    if(L->index!=-1){
      L->index++;
    }
    L->front=node;
  }
  L->length++;
}
//append to the list
void append(List L,void* data){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List reference\n");
    exit(EXIT_FAILURE);
  }
  Node node = newNode(data);
  if(L->length==0){
    L->front=L->back=node;
  } else{
    node->prev = L->back;
    L->back->next=node;
    L->back=node;
  }
  L->length++;
}
//insert before the cursor
void insertBefore(List L, void* data){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List\n");
    exit(EXIT_FAILURE);
  }
  if(L->index<0||L->length<0){
    fprintf(stderr,
            "List Error: calling moveBack() on undefined cursor List\n");
    exit(EXIT_FAILURE);
  }
  if(L->index==0){
    prepend(L,data);
  }
  else{
    Node node  = newNode(data);
    Node temp = L->cursor->prev;
    node->prev = temp;
    node->next  = L->cursor;
    temp->next = node;
    L->cursor->prev  = node;
    L->index++;
    L->length++;
  }
}
//insert after the cursor
void insertAfter(List L, void* data){
  if(L==NULL){
    fprintf(stderr,
            "List Error: calling moveBack() on NULL List \n");
    exit(EXIT_FAILURE);
  }
  if(L->index<0||L->length<0){
    fprintf(stderr,
            "List Error: calling moveBack() on undefined cursor List \n");
    exit(EXIT_FAILURE);
  }
  if(L->index==L->length-1){
    append(L,data);
  } else{
    Node temp = L->cursor->next;
    Node node = newNode(data);
    node->prev = L->cursor;
    L->cursor->next = node;
    node->next = temp;
    temp->prev = node;
    L->length++;
  }
}
//delete the front element of the list
void deleteFront(List L) {
  if (L == NULL) {
    printf("List Error: calling Dequeue() on NULL Queue\n");
    exit(EXIT_FAILURE);
  }
  if (isEmpty(L)) {
    printf("List Error: calling Dequeue on an empty Queue\n");
    exit(EXIT_FAILURE);

  }
  Node N = L->front;
  if (length(L) > 1) {
    N->next->prev = NULL;
    L->front = L->front->next;

  } else {
    L->front = L->back = NULL;
  }
  if (L->index == 0) {
    L->index = -1;
    L->cursor = NULL;
  } else if (L->index != -1) {
    L->index--;
  }
  L->length--;
  freeNode(&N);
}
//delete the back element of the List
void deleteBack(List L) {

  if (L == NULL) {
    printf("List Error: calling deleteBack() on NULL List\n");
    exit(EXIT_FAILURE);

  }
  if (length(L) == 0) {
    printf("List Error: calling deleteBack() on an empty List\n");
    exit(EXIT_FAILURE);

  }

  Node N = L->back;
  //    N->prev->next=NULL;

  if (getindex(L) == length(L) - 1) {
    L->cursor = NULL;
    L->index = -1;
  }

  if (length(L) > 1) {
    N->prev->next=NULL;
    L->back = L->back->prev;

  } else {
    L->front = L->back = NULL;
  }
  L->length--;
  freeNode(&N);
}
//delete the cursor element and still cursor to NULL
void delete(List L){
  if (L == NULL ){
    printf("List Error: calling delete() on NULL List reference\n");
    exit(EXIT_FAILURE);

  }
  if (length(L) == 0){
    printf("List Error: calling delete() on an empty List\n");
    exit(EXIT_FAILURE);

  }
  if (getindex(L) == -1){
    printf("List Error: calling delete() on undefined List cursor\n");
    exit(EXIT_FAILURE);

  }
  Node N = NULL;
  if (getindex(L) == 0) {
    deleteFront(L);
  }
  else if (getindex(L) == length(L) - 1) {
    deleteBack(L);
  }
  else if (length(L) > 0 && getindex(L) > 0){
    N = L->cursor;
    Node prev = L->cursor->prev;
    Node next = L->cursor->next;
    prev->next = L->cursor->next;
    next->prev = L->cursor->prev;
    L->cursor = NULL;
    L->index = -1;
    L->length--;
    freeNode(&N);
  }

}
//print the list to giving out put
//void printList(FILE* out, List L){
//    if(L==NULL){
//        fprintf(stderr,
//                "List Error: calling printList() on NULL List\n");
//        exit(EXIT_FAILURE);
//    }
//    Node N = NULL;
//
//    for (N = L->front;  N!=NULL ; N= N->next) {
//
//        fprintf(out,"%d",N->data);
//    }
//
//}
//List copyList(List L){
//    if(L==NULL){
//        printf("List Error copy NULL list");
//        exit(EXIT_FAILURE);
//    }
//    List ret = newList();
//    if(isEmpty(L)){
//        return ret;
//    }
//
//    Node n = L->front;
//    while(n!=NULL){
//        append(ret,n->data);
//        n = n->next;
//    }
//    return ret;
//}

//int main(){
//    List n = newList();
//    append(n,1);
//    append(n,2);
//    append(n,3);
//    append(n,4);
//    append(n,5);
//    deleteFront(n);
//
//    deleteBack(n);
//    printList(stdout,n);
//    moveNext(n);
//    moveNext(n);
//
//    delete(n);
//    freeList(&n);
//
//}



//
// Created by 董子豪 on 1/30/22.
//
