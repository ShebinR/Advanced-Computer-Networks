#ifndef QUEUE_H
#define QUEUE_H

typedef struct QNode { 
    //int key; 
    uint8_t *data;
    size_t len;
    struct QNode* next; 
} QNode; 
  
typedef struct Queue {
    int capacity; 
    int size;
    pthread_mutex_t lock;
    QNode *front, *rear; 
} Queue;
 
Queue* createQueue(int capacity);
int enQueue(Queue* q, uint8_t *data, size_t len);
int deQueue(Queue* q, QNode *node);
void printQueue(Queue *q);

#endif
