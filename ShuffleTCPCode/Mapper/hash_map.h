#ifndef HASH_MAP_H
#define HASH_MAP_H

typedef struct hash_map hash_map;
typedef struct iterator hash_map_iterator;

hash_map * createHashMap(int size);
hash_map_iterator * createIterator(hash_map *map);
void printMap(hash_map *t);
void insert(hash_map *t, char *key, char *val);
int getNext(hash_map_iterator *itr, char *key, char *val);
void resetIterator(hash_map_iterator *itr);

#endif
