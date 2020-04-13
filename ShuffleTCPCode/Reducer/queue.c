#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h> 
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "queue.h"

/* QUEUE: Implementation
  1 -> 3 -> 5 -> 7 -> NULL
  ^              ^
  |              |
  Front          Rear
*/
  
QNode* newNode(int k) { 
    QNode* temp = (QNode*)malloc(sizeof(QNode)); 
    temp->key = k; 
    temp->next = NULL; 
    return temp; 
} 
  
Queue* createQueue(int capacity) { 
    Queue* q = (Queue*)malloc(sizeof(Queue)); 
    q->capacity = capacity;
    q->size = 0;
    q->front = q->rear = NULL;
    if(pthread_mutex_init(&(q->lock), NULL) != 0) { 
        printf("ERROR: Queue mutex init has failed\n");
        return NULL; 
    }
    return q; 
} 
  
int enQueue(Queue* q, int k) {
    pthread_mutex_lock(&(q->lock));

    if(q->size == q->capacity) {
        printf("INFO: Queue size exceeding capacity! Insertion failed!");
	    
        pthread_mutex_unlock(&(q->lock));
        return -1;
    } 
    QNode* temp = newNode(k);
    /* 1st insert from empty */
    if (q->rear == NULL) { 
        q->front = q->rear = temp;
        q->size++;

	    pthread_mutex_unlock(&(q->lock));
        return 0; 
    } 
  
    q->size++;
    q->rear->next = temp; 
    q->rear = temp;
 
    pthread_mutex_unlock(&(q->lock));
    return 0;
} 
  
int deQueue(Queue* q, QNode *node) { 
    pthread_mutex_lock(&(q->lock));
    
    if (q->front == NULL) {
        printf("INFO: Queue is empty!\n");
    
        // release()
	    pthread_mutex_unlock(&(q->lock));
        return -1; 
    }
  
    // Store previous front and move front one node ahead 
    node = q->front; 
    q->front = q->front->next; 
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
    q->size--;

    pthread_mutex_unlock(&(q->lock));
    return 0;
}

void printQueue(Queue *q) {
    pthread_mutex_lock(&(q->lock));

    QNode *temp = q->front;
    while(temp != NULL) {
        printf("%d -> ", temp->key);
        temp = temp->next;
    }
    printf("\n");

    pthread_mutex_unlock(&(q->lock));
}

