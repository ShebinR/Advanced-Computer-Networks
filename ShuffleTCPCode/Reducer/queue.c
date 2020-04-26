#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h> 
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "queue.h"
#include "communication.h"

/* QUEUE: Implementation
  1 -> 3 -> 5 -> 7 -> NULL
  ^              ^
  |              |
  Front          Rear
*/
  
QNode* newNode(uint8_t *data, size_t len) { 
    QNode* temp = (QNode*)malloc(sizeof(QNode)); 
    temp->data = data;
    temp->len = len;
    temp->next = NULL; 
    return temp; 
} 
  
Queue* createQueue(int capacity) { 
    Queue* q = (Queue*)malloc(sizeof(Queue)); 
    q->capacity = capacity;
    q->size = 0;
    q->front = q->rear = NULL;
    if(pthread_mutex_init(&(q->lock), NULL) != 0) { 
        //printf("ERROR: Queue mutex init has failed\n");
        return NULL; 
    }
    return q; 
} 
  
int enQueue(Queue* q, uint8_t *data, size_t len) {
    pthread_mutex_lock(&(q->lock));

    if(q->size == q->capacity) {
        //printf("COMMUNICATION THREAD: Queue size exceeding capacity! Insertion failed!\n");
	    
        pthread_mutex_unlock(&(q->lock));
        return -1;
    } 
    QNode* temp = newNode(data, len);
    /* 1st insert from empty */
    if (q->rear == NULL) { 
        q->front = q->rear = temp;
        q->size++;

        //printf("COMMUNICATION THREAD: Enqueue done!\n");
	    pthread_mutex_unlock(&(q->lock));
        return 0; 
    } 
  
    q->size++;
    q->rear->next = temp; 
    q->rear = temp;
 
    //printf("COMMUNICATION THREAD: Enqueue done!\n");
    pthread_mutex_unlock(&(q->lock));
    return 0;
} 
  
QNode* deQueue(Queue* q, QNode **node) { 
    pthread_mutex_lock(&(q->lock));
    
    if (q->front == NULL) {
        ////printf("INFO: Queue is empty!\n");
    
        // release()
	    pthread_mutex_unlock(&(q->lock));
        return NULL; 
    }
  
    // Store previous front and move front one node ahead 
    QNode *temp = q->front; 
    q->front = q->front->next; 
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
    q->size--;
    //printf("\tGROUPER THREAD: Dequeue done!\n");
    pthread_mutex_unlock(&(q->lock));
    return temp;
}

void printQueue(Queue *q) {
    pthread_mutex_lock(&(q->lock));

    QNode *temp = q->front;
    if(temp == NULL)
        //printf("INFO: Queue is empty!");
    while(temp != NULL) {
        int no_of_records = 0;
        char **messages = deserializeChunkFetchReply(temp->data, temp->len, &no_of_records);
        if(messages != NULL) {
            //for(int i = 0; i < no_of_records; i++) {
              //  if(i > 0)
                    //printf(", ");
                //printf("%s", messages[i]);
            //}
            //printf("\n");
        }
        temp = temp->next;
    }
    //printf("\n");

    pthread_mutex_unlock(&(q->lock));
}

