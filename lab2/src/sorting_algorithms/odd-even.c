#include <stdio.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>
#include <stdbool.h>

#include "sorting.h"

/* 
   odd-even sort -- sequential, parallel -- 
*/


void sequential_oddeven_sort (uint64_t *T, const uint64_t size)
{
    /* TODO: sequential implementation of odd-even sort */
    int sorted = 0;
    int i;
    while(sorted != 1) {
        sorted = 1;
        for(i = 0; i < size; i+=2) {
            if(T[i] > T[i+1]){
                sorted = 0;
                int save = T[i];
                T[i] = T[i+1];
                T[i+1] = save;
            }
        }
        for(i = 1; i<size; i+=2){
            if(T[i] > T[i+1]){
                sorted = 0;
                int save = T[i];
                T[i] = T[i+1];
                T[i+1] = save;
            }
        }
    }
    return ;
}


void parallel_oddeven_sort (uint64_t *T, const uint64_t size)
{

    /* TODO: parallel implementation of odd-even sort */ 
    int sorted = 0, sorted1 = 0;
    int i;

    while(sorted != 1 || sorted1 != 1) {
        sorted = 1;
        sorted1 = 1;
        #pragma omp parallel
        {
            #pragma omp for
            for(i = 0; i < size; i+=2) {
                if(T[i] > T[i+1]){
                    sorted = 0;
                    int save = T[i];
                    T[i] = T[i+1];
                    T[i+1] = save;
                }
            }
            #pragma omp for
            for(i = 1; i<size; i+=2){
                if(T[i] > T[i+1]){
                    sorted1 = 0;
                    int save = T[i];
                    T[i] = T[i+1];
                    T[i+1] = save;
                }
            }
        }
    }

    return ;
}


int main (int argc, char **argv)
{
    uint64_t start, end;
    uint64_t av ;
    unsigned int exp ;

    /* the program takes one parameter N which is the size of the array to
       be sorted. The array will have size 2^N */
    if (argc != 2)
    {
        fprintf (stderr, "odd-even.run N \n") ;
        exit (-1) ;
    }

    uint64_t N = 1 << (atoi(argv[1])) ;
    /* the array to be sorted */
    uint64_t *X = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

    printf("--> Sorting an array of size %u\n",N);
#ifdef RINIT
    printf("--> The array is initialized randomly\n");
#endif
    

    for (exp = 0 ; exp < NBEXPERIMENTS; exp++){
#ifdef RINIT
        init_array_random (X, N);
#else
        init_array_sequence (X, N);
#endif
        
      
        start = _rdtsc () ;
        
        sequential_oddeven_sort (X, N) ;
     
        end = _rdtsc () ;
        experiments [exp] = end - start ;

        /* verifying that X is properly sorted */
#ifdef RINIT
        if (! is_sorted (X, N))
        {
            fprintf(stderr, "ERROR: the sequential sorting of the array failed\n") ;
            print_array (X, N) ;
            exit (-1) ;
	}
#else
        if (! is_sorted_sequence (X, N))
        {
            fprintf(stderr, "ERROR: the sequential sorting of the array failed\n") ;
            print_array (X, N) ;
            exit (-1) ;
	}
#endif
    }

    av = average_time() ;  

    printf ("\n odd-even serial \t\t\t %.2lf Mcycles\n\n", (double)av/1000000) ;

  
    for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
    {
#ifdef RINIT
        init_array_random (X, N);
#else
        init_array_sequence (X, N);
#endif
        
        start = _rdtsc () ;


        parallel_oddeven_sort (X, N) ;

     
        end = _rdtsc () ;
        experiments [exp] = end - start ;

        /* verifying that X is properly sorted */
#ifdef RINIT
        if (! is_sorted (X, N))
        {
            fprintf(stderr, "ERROR: the parallel sorting of the array failed\n") ;
            exit (-1) ;
	}
#else
        if (! is_sorted_sequence (X, N))
        {
            fprintf(stderr, "ERROR: the parallel sorting of the array failed\n") ;
            exit (-1) ;
	}
#endif
                
        
    }
    
    av = average_time() ;  
    printf ("\n odd-even parallel \t\t %.2lf Mcycles\n\n", (double)av/1000000) ;
  
    /* print_array (X, N) ; */

    /* before terminating, we run one extra test of the algorithm */
    uint64_t *Y = (uint64_t *) malloc (N * sizeof(uint64_t)) ;
    uint64_t *Z = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

#ifdef RINIT
    init_array_random (Y, N);
#else
    init_array_sequence (Y, N);
#endif

    memcpy(Z, Y, N * sizeof(uint64_t));

    sequential_oddeven_sort (Y, N) ;
    parallel_oddeven_sort (Z, N) ;

    if (! are_vector_equals (Y, Z, N)) {
        fprintf(stderr, "ERROR: sorting with the sequential and the parallel algorithm does not give the same result\n") ;
        exit (-1) ;
    }


    free(X);
    free(Y);
    free(Z);
    
}
