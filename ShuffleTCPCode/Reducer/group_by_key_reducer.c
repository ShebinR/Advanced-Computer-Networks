#include <stdio.h>
#include <pthread.h>

#include "thread_info.h"
#include "hash_map_group_by_key.h"
#include "queue.h"
#include "communication.h"

void groupByKeyReducer(void *input) {
    pthread_t t = pthread_self();

    char *thread_name = ((thread_info_grouper *)input)->thread_name;
    Queue *queue = ((thread_info_grouper *)input)->result_queue;
    hash_map_group_by_key *map = ((thread_info_grouper *)input)->map;
    int *mapper_status = ((thread_info_grouper *)input)->mapper_status;

    printf("INFO: Thread ID:: %d Name : %s\n", (int)t, thread_name);
    //sleep(15);
    while(1) {
        QNode *node = NULL;
        int ret = -1;
        do {
            node = deQueue(queue, &node);
            //sleep(1);
            if(*mapper_status == 1)
                break;
        } while(node == NULL);
        if(*mapper_status == 1 & node == NULL) {
            printf("\tGROUPER THREAD: Mappers done & no more data in queue!\n");
            break;
        }

        int no_of_records = 0;
        printf("\tGROUPER THREAD: Deserializing reply!\n");
        char **messages = deserializeChunkFetchReply(node->data, node->len, &no_of_records);
        if(messages != NULL) {
            printf("\tGROUPER THREAD: Adding to result map!\n");
            for(int i = 0; i < no_of_records; i += 2) {
                printf("\t%s -> %s ", messages[i], messages[i+1]);
                insert(map, messages[i], messages[i+1]);
                free(messages[i]);
                free(messages[i+1]);
            }
            fflush(stdout);
        } else {
            printf("\tGROUPER THREAD: Error!! deserializing reply!\n");
        }
	    printf("\n");
        free(node);
    }
    printf("\n");
    printf("\tGROUPER THREAD: Group by done\n");
}
