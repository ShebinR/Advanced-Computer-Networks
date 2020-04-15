#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

typedef struct val_node {
    char val[VAL_SIZE];
    struct val_node *next;
} val_node;

typedef struct key_node {
    char key[KEY_SIZE];
    struct val_node *val_node;
    struct key_node *next;
} key_node;

typedef struct hash_map_group_by_key {
    int size;
    key_node **head;
} hash_map_group_by_key;

//typedef struct iterator {
  //  hash_map *map;
    //int index;
   // node *curr;
//} hash_map_iterator;

hash_map_group_by_key * createHashMapGroupByKey(int size) {
    hash_map_group_by_key *map = (hash_map_group_by_key *) malloc (sizeof(hash_map_group_by_key));
    map->size = size;
    map->head = (key_node **) malloc (sizeof(key_node) * size);
    for(int i = 0; i < size; i++)
        map->head[i] = NULL;

    return map;
}

/*hash_map_iterator * createIterator(hash_map *map) {
    hash_map_iterator *itr = (hash_map_iterator *) malloc (sizeof(hash_map_iterator));
    itr->map = map;
    itr->index = 0;
    itr->curr = map->head[itr->index];
    while(itr->curr == NULL) {
        itr->index++;
        if(itr->index == itr->map->size) 
            break;
        itr->curr = itr->map->head[itr->index];
    }
    return itr; 
} 

int getNext(hash_map_iterator *itr, char *key, char *val) {
    if(itr->curr == NULL) {
        return -1;
    }
    strcpy(key, itr->curr->key);
    strcpy(val, itr->curr->val);

    itr->curr = itr->curr->next;
    while(itr->curr == NULL) {
        itr->index++;
        if(itr->index == itr->map->size) {
            // Reached End
            return 1;
        }
        itr->curr = itr->map->head[itr->index];
    }
    return 0;
} */

int hashCode(hash_map_group_by_key *m, char *key) {
    if(key[0] < 0)
        return -(key[0] % m->size);
    int sum = 0, i = 0;
    while(key[i] != '\0') {
        //printf("%c \n", key[i]);
        sum += (int) key[i++];
        //printf("Sum : %d\n", sum);
    }
    return (sum % m->size);
}

void printMap(hash_map_group_by_key *t) {
    printf("Hash Map :: \n");
    for(int i = 0; i < t->size; i++) {
        key_node *curr = t->head[i];
        printf("%d :: ", i);
        while(curr != NULL) {
            printf("(%s : [", curr->key);
            val_node *val_curr = curr->val_node;
            while(val_curr != NULL) {
                printf("%s, ", val_curr->val);
                val_curr = val_curr->next;
            }
            printf("] ) ->");
            curr = curr->next;
        }
        printf("NULL\n");
    }
}

void insert(hash_map_group_by_key *t, char *key, char *val) {
    int pos = hashCode(t, key);
    key_node *curr_key_head;
    if(t->head[pos] == NULL) {
        key_node *new_key_node = (key_node *) malloc (sizeof(key_node));
        strcpy(new_key_node->key, key); 
        t->head[pos] = new_key_node;
        curr_key_head = t->head[pos];
    } else {
        key_node *prev = NULL;
        curr_key_head = t->head[pos];
        while(curr_key_head != NULL && strcmp(curr_key_head->key, key) != 0) {
            prev = curr_key_head;
            curr_key_head = curr_key_head->next;
        }
        if(curr_key_head == NULL) {
            //printf("Existing key not found!\n");
            curr_key_head = prev;
            curr_key_head->next = (key_node *) malloc (sizeof(key_node));
            curr_key_head = curr_key_head->next;
            strcpy(curr_key_head->key, key);
        } else {
            //printf("Existing key found!\n");
        }

    }
        
    val_node *newValNode = (val_node *) malloc (sizeof(val_node));
    strcpy(newValNode->val, val);

    if(curr_key_head->val_node == NULL) { 
        curr_key_head->val_node = newValNode;
        return;
    }
    val_node *temp = curr_key_head->val_node;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newValNode;
}

/*
void rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

int main() {
    hash_map_group_by_key *map = createHashMapGroupByKey(10);

    insert(map, "sheb", "hello");
    insert(map, "sheb", "hello2");
    insert(map, "ten", "10");
    insert(map, "plat", "test");
    for(int i = 0; i < 10; i++)
        insert(map, "lpat", "ni");
    for(int i = 0; i < 10; i++)
        insert(map, "lapt", "ni");
    insert(map, "gat", "test");
    insert(map, "nic", "tell");
    insert(map, "net", "tell");

    for(int i = 0; i < 100; i++) {
        char *key = (char *) malloc(sizeof(char) * KEY_SIZE);
        rand_string(key, KEY_SIZE);
        char *val = (char *) malloc(sizeof(char) * VAL_SIZE);
        rand_string(val, VAL_SIZE);
        insert(map, key, val);

        free(key); free(val);
    }

    printMap(map);
}

*/
