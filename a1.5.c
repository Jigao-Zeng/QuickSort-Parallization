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
#include <math.h>


#define SIZE    10


struct block {
    int size;
    int *data;
};

int min = 0;

int proceeNum = 1;

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

/* Step 6 use recursive call to create multi process*/
void multi_quick_sort(struct block my_data){
    
    int pivot_pos = split_on_pivot(my_data);
    struct block left_side, right_side;
    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

	pid_t child_left = -1;
	pid_t child_right = -1;
	
	int file_dec_left[2];
	int file_dec_right[2];
	
	int pipe_status_left = pipe(file_dec_left);
	int pipe_status_right = pipe(file_dec_right);
    

    child_left = fork();
    if(child_left < 0){
        printf("Failure to create left block process.\n");
		exit(1);
    }else if(child_left ==0){
        // child process
        close(file_dec_left[0]);
        proceeNum++;
        if(left_side.size < min){
            
            quick_sort(left_side);
        }else{
            multi_quick_sort(left_side);
        }
        write(file_dec_left[1],left_side.data,left_side.size*sizeof(int));
        exit(0);
    }

    child_right = fork();
    if(child_right < 0){
        printf("Failure to create left block process.\n");
		exit(1);
    }else if(child_right ==0){
        // child process
        close(file_dec_right[0]);
        proceeNum++;
        if(right_side.size < min){
           
            quick_sort(right_side);
        }
        else{
             multi_quick_sort(right_side);
        }
            
        write(file_dec_right[1],right_side.data,right_side.size*sizeof(int));
        exit(0);

    }

    close(file_dec_left[1]);
    close(file_dec_right[1]);
    read(file_dec_left[0],left_side.data,left_side.size*sizeof(int));
    read(file_dec_right[0],right_side.data,right_side.size*sizeof(int));

    
    
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

    if(start_block.size > 100000){
        min = start_block.size/50;
    }else{
        min = start_block.size;
    }
    
    struct tms start_times, finish_times;
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    multi_quick_sort(start_block);
    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);

    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    free(start_block.data);
    exit(EXIT_SUCCESS);
}