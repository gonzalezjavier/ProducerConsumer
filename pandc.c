#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>


int* buffer;
int* producerArray;
int* consumerArray;
int numOfBuffers,numOfProducers,numOfConsumers,eachProdItems,pTime,cTime;


struct thread_info{
    pthread_t tid;
    int readable_id;
};

/**
 * for the 2 functions below, look
 * at the process slides/video for 
 * sample code that can be used.
 */
/* 
 * Function to remove item.
 * Item removed is returned
 */
int grab_item()
{
    
}

/* 
 * function to put item
 * into shared resource 
 * so it can be consumed
 */
void put_item(int item)
{
    
}

void *producer(void* arg){
    //this will call the put_item
}

void *consumer(void* arg){
    //this will call the get_item
}

int main(int argc, char* argv[]) 
{
    //this checks if there is the proper command line arguments
    if (argc!=7)
    {
        printf("Invalid use of command line arguments\n"
        "Valid usage: ./pandc N P C X Ptime Ctime\n"
        "N = number of buffers\n"
        "P = number of producers\n"
        "C = number of consumers\n"
        "X = number of items produced by each producer thread\n"
        "Ptime = time spent busy waiting between produced items in seconds\n"
        "Ctime = time spent busy waiting between consumed items in seconds\n");
        return -1;
    }
    
    //create time variables to keep track of time
    struct timespec start_time;
    struct timespec end_time;


    time_t seconds;
    long nano_seconds;


    //read all args and store
    
    numOfBuffers = atoi(argv[1]); 
    numOfProducers =  atoi(argv[2]);
    numOfConsumers =  atoi(argv[3]);
    eachProdItems =  atoi(argv[4]);
    pTime =  atoi(argv[5]);
    cTime =  atoi(argv[6]);
    
    //print timestamp and all args

    printf(
        "\t                        Number of buffers:  %d\n"
        "\t                      Number of producers:  %d\n"
        "\t                      Number of consumers:  %d\n"
        "\tNumber of items produced by each producer:  %d\n"
        "\tNumber of items consumed by each consumer:  %d\n",numOfBuffers,numOfProducers,numOfConsumers,eachProdItems,(numOfProducers*eachProdItems/numOfConsumers));

    //allocate memory
    buffer= malloc(numOfBuffers*sizeof(int));
    producerArray =  malloc((numOfProducers*eachProdItems)*sizeof(int));
    consumerArray =  malloc((numOfProducers*eachProdItems)*sizeof(int));
    

    /**
     * we spawn all threads
     * join all threads
     * 
     */
    struct thread_info producerThread[numOfProducers];
    struct thread_info consumerThread[numOfConsumers];
    //spawns all producer threads and assigned readable ids
    for(int i=0;i<numOfProducers;i++){
        producerThread[i].readable_id=i+1;
        pthread_create(&producerThread[i].tid,NULL,producer,(void*)&producerThread[i]);
    }
    //spawns all consumer threads and assigns readable ids
    for(int i=0;i<numOfConsumers;i++){
        consumerThread[i].readable_id=i+1;
        pthread_create(&consumerThread[i].tid,NULL,producer,(void*)&consumerThread[i]);
    }


    //run test strategy for proof




}
