#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

static const long Num_To_Add = 1000000000;
static const double Scale = 10.0 / RAND_MAX;

long add_serial(const char *numbers) {
    long sum = 0;
    for (long i = 0; i < Num_To_Add; i++) {
        sum += numbers[i];
    }
    return sum;
}

long add_parallel(const char *numbers) {
    long sum = 0;

    long num = omp_get_max_threads(); //Max number of threads
    long chunk = Num_To_Add / num; //Number of indexes of the array that each thread will have to cover
    long start, end;

    #pragma omp parallel num_threads(num) private(start,end) reduction(+:sum) //Creates a parallel region and reduces the sum of the random numbers to 'sum'
    {   //start and end are private so that each thread has its own copy

        start = omp_get_thread_num() * chunk; //start represents the starting index in the for loop for the thread
        end = start + chunk; //end represents the ending index (+1) for the thread in the for loop for the thread

        for (long i = start; i < end; i++) {
            sum += numbers[i];
        }
    }

    return sum;
}

int main() {
    char *numbers = malloc(sizeof(long) * Num_To_Add);

    long chunk_size = Num_To_Add / omp_get_max_threads();
    #pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;
        for (long i = chunk_start; i < chunk_end; i++) {
            numbers[i] = (char) (rand_r(&seed) * Scale);
        }
    }

    struct timeval start, end;

    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    long sum_s = add_serial(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    long sum_p = add_parallel(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Sum serial: %ld\nSum parallel: %ld", sum_s, sum_p);

    free(numbers);
    return 0;
}

