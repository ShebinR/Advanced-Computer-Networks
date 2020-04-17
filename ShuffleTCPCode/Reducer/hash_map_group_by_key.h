#ifndef HASH_MAP_GROUP_BY_KEY
#define HASH_MAP_GROUP_BY_KEY

typedef struct hash_map_group_by_key hash_map_group_by_key;
hash_map_group_by_key * createHashMapGroupByKey(int size);
void insert(hash_map_group_by_key *t, char *key, char *val);
void printMap(hash_map_group_by_key *t);

#endif
