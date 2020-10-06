/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: Jigao Zeng
    UPI: jzen557

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/times.h>
#include <pthread.h>



#define SIZE    10

struct block {
    int size;
    int *data;
};

int is_done = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct block both_block;



void print_data(struct block my_data) {
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}







/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block my_data) {
    int right = my_data.size - 1;
    int left = 0;
    int pivot = my_data.data[right];
    while (left < right) {
        int value = my_data.data[right - 1];
        if (value > pivot) {
            my_data.data[right--] = value;
        } else {
            my_data.data[right - 1] = my_data.data[left];
            my_data.data[left++] = value;
        }
    }
    my_data.data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void quick_reuse_sort(struct block my_data) {
    if (my_data.size < 2)
        return;
    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;
    
    pthread_mutex_lock(&lock);
    int size=both_block.size;
    pthread_mutex_unlock(&lock);

    if (size == 0){
        pthread_mutex_lock(&lock);
        both_block.size=left_side.size;
        both_block.data=left_side.data;
        pthread_mutex_unlock(&lock);
         pthread_cond_signal(&cond);
         quick_reuse_sort(right_side);
    }else{
        quick_reuse_sort(left_side);
        quick_reuse_sort(right_side);
    }
    
/*
    quick_sort(left_side);
    quick_sort(right_side);*/
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block my_data) {
    bool sorted = true;
    for (int i = 0; i < my_data.size - 1; i++) {
        if (my_data.data[i] > my_data.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block my_data) {
    srand(1); // the same random data seed every time
    for (int i = 0; i < my_data.size; i++) {
        my_data.data[i] = rand() % 1000;
    }
}
/* Quick sort the data. */
void quick_sort(struct block my_data) {
    if (my_data.size < 2)
        return;
    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

    quick_sort(left_side);
    quick_sort(right_side);
}

void work(struct block data){
    while (1) {
        int done=0;
        pthread_mutex_lock(&lock);
        done=is_done;
        pthread_mutex_unlock(&lock);

        if (done == 1) {
            return;
        }

        pthread_mutex_lock(&lock);
        while (both_block.size==0){

            pthread_cond_wait(&cond, &lock);

        }

        data.size=both_block.size;
        data.data=both_block.data;
        pthread_mutex_unlock(&lock);

        quick_sort(data);

        pthread_mutex_lock(&lock);
        both_block.size = 0;
        pthread_mutex_unlock(&lock);
    }
}
   


    

    
   
    
    







int main(int argc, char *argv[]) {
	long size;
    
    
    
	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}

    struct block start_block;
    start_block.size = size;
    start_block.data = (int *)calloc(size, sizeof(int));
    if (start_block.data == NULL) {
        printf("Problem allocating memory.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(start_block);

    if (start_block.size < 1001)
        print_data(start_block);

    pthread_t second_thread;

    pthread_create(&second_thread, NULL, (void *)work, (void *)&both_block);

    struct tms start_times, finish_times;

    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    quick_reuse_sort(start_block);
 
    pthread_mutex_lock(&lock);

    is_done = 1;
    pthread_mutex_unlock(&lock);
    pthread_join(second_thread, NULL);  
    
    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    free(start_block.data);
    exit(EXIT_SUCCESS);
}