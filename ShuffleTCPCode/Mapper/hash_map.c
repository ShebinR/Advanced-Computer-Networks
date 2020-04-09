#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Constants.h"

typedef struct node {
    char key[KEY_SIZE];
    char val[VAL_SIZE];
    struct node *next;
} node;

typedef struct hash_map {
    int size;
    node **head;
} hash_map;

hash_map * createHashMap(int size) {
    hash_map *map = (hash_map *) malloc (sizeof(hash_map));
    map->size = size;
    map->head = (node **) malloc (sizeof(node) * size);
    for(int i = 0; i < size; i++)
        map->head[i] = NULL;

    return map;
}

int hashCode(hash_map *t, char *key) {
    if(key[0] < 0)
        return -(key[0] % t->size);
    int sum = 0, i = 0;
    while(key[i] != '\0') {
        //printf("%c \n", key[i]);
        sum += (int) key[i++];
        //printf("Sum : %d\n", sum);
    }
    return (sum % t->size);
}

void printMap(hash_map *t) {
    printf("Hash Map :: \n");
    for(int i = 0; i < t->size; i++) {
        node *curr = t->head[i];
        printf("%d :: ", i);
        while(curr != NULL) {
            printf("(%s : %s) -> ", curr->key, curr->val);
            curr = curr->next;
        }
        printf("NULL\n");
    }
}

void insert(hash_map *t, char *key, char *val) {
    int pos = hashCode(t, key);
    node *list = t->head[pos];
    node *newNode = (node *) malloc (sizeof(node));
    node *temp = list;
    while (temp != NULL) {
        if(strcmp(temp->key, key) == 0) {
            strcpy(temp->val, val);
            return;
        }
        temp = temp->next;
    }
    strcpy(newNode->key, key);
    strcpy(newNode->val, val);
    newNode->next = list;
    t->head[pos] = newNode;
}

