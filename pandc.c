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

//array to be added and removed from
int* buffer;
//for circular array
int out = 0;
int in = 0;
//array used for comparing at end of program
int* producerArray;
int* consumerArray;

//all args
int numOfBuffers,numOfProducers,numOfConsumers,eachProdItems,pTime,cTime,eachConsumerItems;
int overconsume = 0;
int overconsumeAmount = 0;

//items that will be added
int itemCounter=1;
//to keep track of index/size of checking arrays
int prodIndex=0;
int consIndex=0;

//locking semaphores
sem_t binarySem, consumptionProtectionSem, overproductionProtectionSem;

struct thread_info{
    pthread_t tid;
    int readable_id;
};


/* 
 * Function to remove item.
 * Item removed is returned
 */
int grab_item()
{
    int item = buffer[out];
    out = (out + 1) % numOfBuffers;
    return item;
}

/* 
 * function to put item
 * into shared resource 
 * so it can be consumed
 */
void put_item(int item)
{
    buffer[in] = item;
    in = (in +1) % numOfBuffers;
}

void *producer(void* arg){
    //this will call the put_item
    struct thread_info* producerThread = (struct thread_info*)arg;
    for(int i=0; i < eachProdItems; i++){
        //grab overproduction semaphore to make sure buffer is not full
        sem_wait(&overproductionProtectionSem);
        //grab the binary semaphore that protects main data 
        sem_wait(&binarySem);
        put_item(itemCounter);
        producerArray[prodIndex++] = itemCounter;
        printf("%8d was produced by producer ->%8d\n",itemCounter, producerThread->readable_id);
        itemCounter++;
        //release data protector semaphore
        sem_post(&binarySem);
        //allow consumers to start consumption of produced items
         sem_post(&consumptionProtectionSem);
        sleep(pTime);
    }
}

void *consumer(void* arg){
    //this will call the get_item
    struct thread_info* consumerThread = (struct thread_info*)arg;
    //overconsume not needed
    if(!overconsume){
        for(int i = 0; i < eachConsumerItems; i++){
            //wait for there to be items to consume
            sem_wait(&consumptionProtectionSem);
            //waits for the data protector to be available
            sem_wait(&binarySem);
            int grabbedItem = grab_item();
            consumerArray[consIndex++] = grabbedItem;
            printf("%8d was consumed by consumer ->%8d\n", grabbedItem, consumerThread->readable_id);
            //release data protector
            sem_post(&binarySem);
            //release a consumed buffer to be written to by producer
            sem_post(&overproductionProtectionSem);
            sleep(cTime);
        }
    } else {
        //overconsumption needed
        overconsume=0;
        for(int i = 0; i < eachConsumerItems+overconsumeAmount; i++){
            //wait for there to be items to consume
            sem_wait(&consumptionProtectionSem);
            //waits for the data protector to be available
            sem_wait(&binarySem);
            int grabbedItem = grab_item();
            consumerArray[consIndex++] = grabbedItem;
            printf("%8d was consumed by consumer ->%8d\n", grabbedItem, consumerThread->readable_id);
            //release data protector
            sem_post(&binarySem);
            //release a consumed buffer to be written to by producer
            sem_post(&overproductionProtectionSem);
            sleep(cTime);
        }
    }
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
    eachConsumerItems = (numOfProducers*eachProdItems) / numOfConsumers;

    //takes care of overconsumption for consumer threads
    overconsume = (numOfProducers * eachProdItems) % numOfConsumers ? 1 : 0;
    if(overconsume){
        overconsumeAmount = (numOfProducers * eachProdItems) - (numOfConsumers * eachConsumerItems);
    }

    //allocate memory
    buffer= malloc(numOfBuffers*sizeof(int));
    producerArray =  malloc((numOfProducers*eachProdItems)*sizeof(int));
    consumerArray =  malloc((numOfProducers*eachProdItems)*sizeof(int));
    
    //print timestamp and all args
    clock_gettime(0, &start_time);
    printf("Current Time: %s\n\n",ctime(&start_time.tv_sec));

    printf(
        "                        Number of buffers:  %d\n"
        "                      Number of producers:  %d\n"
        "                      Number of consumers:  %d\n"
        "Number of items produced by each producer:  %d\n"
        "Number of items consumed by each consumer:  %d\n"
        "                         Over consume on?:  %d\n"
        "                     Over consume ammount:  %d\n"
        "      Time each Producer sleeps (seconds):  %d\n"
        "      Time each Consumer sleeps (seconds):  %d\n\n"
        ,numOfBuffers,numOfProducers,numOfConsumers,eachProdItems,eachConsumerItems,overconsume,overconsumeAmount,pTime,cTime);


    /**
     * we spawn all threads
     * join all threads
     * 
     */
    struct thread_info producerThread[numOfProducers];
    struct thread_info consumerThread[numOfConsumers];
    //initialize semaphores
    sem_init(&binarySem, 0, 1);
    sem_init(&consumptionProtectionSem, 0, 0); //set to 0 so that consumer cannot start consuming until producers have produced
    sem_init(&overproductionProtectionSem, 0, numOfBuffers); //counting semaphore so that producer doesn't overproduce (overwrite)

    //spawns all producer threads and assigned readable ids
    for(int i=0;i<numOfProducers;i++){
        producerThread[i].readable_id=i+1;
        pthread_create(&producerThread[i].tid,NULL,producer,(void*)&producerThread[i]);
    }
    //spawns all consumer threads and assigns readable ids
    for(int i=0;i<numOfConsumers;i++){
        consumerThread[i].readable_id=i+1;
        pthread_create(&consumerThread[i].tid,NULL,consumer,(void*)&consumerThread[i]);
    }

    //joins all producer threads
    for(int i=0;i<numOfProducers;i++){
        pthread_join(producerThread[i].tid,NULL);
        printf("Producer Thread joined:\t%d\n",producerThread[i].readable_id);
    }
    //joins all consumer threads
    for(int i=0;i<numOfConsumers;i++){
        pthread_join(consumerThread[i].tid,NULL);
        printf("Consumer Thread joined:\t%d\n",consumerThread[i].readable_id);
    }
    
    //Print out timestamp
    clock_gettime(0, &end_time);
    printf("\nCurrent Time: %s\n",ctime(&end_time.tv_sec));

    //run test strategy for proof
    int matcher = 0;
    printf("Producer Array    |  Consumer Array\n");
    for (int i = 0; i < prodIndex; i++)
    {
        if (producerArray[i] == consumerArray[i])
        {
            matcher +=1;
        }
        
        printf("%-18d|  %-15d\n",producerArray[i],consumerArray[i]);
    }
    if(matcher == prodIndex && matcher == consIndex) {
        printf("Producer and Consumer Arrays Match!\n");
    } else {
        printf("Producer and Consumer Arrays do NOT Match!\n");
    }
    
    //print elapsed time
    seconds = end_time.tv_sec - start_time.tv_sec;
    nano_seconds = end_time.tv_nsec - start_time.tv_nsec;

    if (end_time.tv_nsec < start_time.tv_nsec)
    {
        seconds--;
        nano_seconds+=1000000000L;
    }
    printf("\nTotal runtime: %ld.%09ld seconds\n",seconds, nano_seconds);
    
    return 0;

}
