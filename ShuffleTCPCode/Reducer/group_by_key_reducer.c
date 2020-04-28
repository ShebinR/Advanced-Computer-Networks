#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h> 

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
    stats_grouper *stats_g = ((thread_info_grouper *)input)->stats_g;
    int core_id = ((thread_info_grouper *)input)->core_id;

    /* SETTING CPU AFFINITY */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    int set_result = pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset);
    if(set_result != 0) {
        printf("ERROR: CPU AFINITY COULD NOT BE SET! exiting ");
        exit(0);
    }

    printf("INFO: Thread ID:: %d Name : %s Core ID : %d\n", (int)t, thread_name, core_id);
    int index = 0;
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
        struct timeval start_t, end_t;
        gettimeofday(&start_t, NULL);
        //clock_t start = clock();
        //stats_g->d_start[index] = start;
        char **messages = deserializeChunkFetchReply(node->data, node->len, &no_of_records);
        if(messages != NULL) {
            printf("\tGROUPER THREAD: Deserialize reply : %d\n", index);
            //fflush(stdout);
            for(int i = 0; i < no_of_records; i += 2) {
                //printf("\t%s -> %s ", messages[i], messages[i+1]);
                insert(map, messages[i], messages[i+1]);
                free(messages[i]);
                free(messages[i+1]);
            }
            //fflush(stdout);
        } else {
            printf("\tGROUPER THREAD: Error..!! Deserializing reply : %d\n", index);
        }
	    //printf("\n");
        free(node);
        //printf("Updating clock! %d\n", index);
        //fflush(stdout);
        //clock_t end = clock();
        gettimeofday(&end_t, NULL); 

        //stats_g->d_end[index] = end;
        //stats_g->d_diff[index] = end - start;
        //printf("End in D %d stats : %ld Diff : %ld\n", index, stats_g->d_end[index], stats_g->d_diff[index]);

        double time_taken;
        time_taken = (end_t.tv_sec - start_t.tv_sec) * 1e6; 
        time_taken = (time_taken + (end_t.tv_usec -  
                              start_t.tv_usec)) * 1e-6;
        //printf("TT: %f \n", time_taken);
        //fflush(stdout);
        stats_g->per_tt[index] = time_taken * 1000;
        //printf("Written\n");
        //fflush(stdout);
        index++;
    }
    printf("\n");
    printf("\tGROUPER THREAD: Group by done\n");
}
