#ifndef HASH_MAP_H
#define HASH_MAP_H

typedef struct hash_map hash_map;

hash_map * createHashMap(int size);
void printMap(hash_map *t);
void insert(hash_map *t, char *key, char *val);

#endif
