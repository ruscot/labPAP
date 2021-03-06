Skip to content
Search or jump to…

Pull requests
Issues
Marketplace
Explore
 
@vinsonc 
ruscot
/
labPAP
1
00
Code
Issues
Pull requests
Actions
Projects
Wiki
Security
Insights
labPAP/lab2/src/sorting_algorithms/bubble.c
@ruscot
ruscot ajout des fichiers du lab2
Latest commit 2474e3c 2 days ago
 History
 1 contributor
214 lines (167 sloc)  5.33 KB
  
#include <stdio.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>

#include "sorting.h"

/* 
   bubble sort -- sequential, parallel -- 
*/

void sequential_bubble_sort (uint64_t *T, const uint64_t size)
{
    /* TODO: sequential implementation of bubble sort */ 
    int sorted = 0;
    do {
        sorted = 1;
        for(int i = 0; i < size-1; i++){
            if(T[i] > T[i+1]){
                sorted = 0;
                uint64_t save = T[i];
                T[i] = T[i+1];
                T[i+1] = save;
            }
        }
    }while (sorted == 0);
    
    return ;
}



void parallel_bubble_sort (uint64_t *T, const uint64_t size)
{
    /* TODO: parallel implementation of bubble sort */
    int sorted = 0;
    //Multiple de 2
    int numberOfChunks = 16;
    
    int sizeOfChunks = size/numberOfChunks;
    
    while(sorted == 0){
        //Sort of each chunks
        sorted = 1;
        #pragma omp for
        for(int i = 0; i < numberOfChunks; i++){
            if(i < numberOfChunks) {
                sequential_bubble_sort(T + i * sizeOfChunks, sizeOfChunks);
            } else {
                sequential_bubble_sort(T + i * sizeOfChunks, size - i * sizeOfChunks);
            }   
            if(i + 1 < numberOfChunks) {
                if(T[sizeOfChunks + i *sizeOfChunks - 1] > T[sizeOfChunks + i *sizeOfChunks]){
                    uint64_t save = T[sizeOfChunks + i *sizeOfChunks - 1];
                    T[sizeOfChunks + i *sizeOfChunks - 1] = T[sizeOfChunks + i *sizeOfChunks];
                    T[sizeOfChunks + i *sizeOfChunks] = save;
                    sorted = 0;
                }
            }
        }
        #pragma omp barrier
        /*//Check if chunks are sorted
        sorted = 1;
        //#pragma omp parallel for default(none),shared(T,numberOfChunks, sizeOfChunks, sorted)
        //#pragma omp for
        for(int i = 0; i < numberOfChunks; i++){
            if(i + 1 < numberOfChunks) {
                if(T[sizeOfChunks + i *sizeOfChunks - 1] > T[sizeOfChunks + i *sizeOfChunks]){
                    uint64_t save = T[sizeOfChunks + i *sizeOfChunks - 1];
                    T[sizeOfChunks + i *sizeOfChunks - 1] = T[sizeOfChunks + i *sizeOfChunks];
                    T[sizeOfChunks + i *sizeOfChunks] = save;
                    sorted = 0;
                }
            } 
        }
        //#pragma omp barrier*/
    
        
    }

    return;
}

void parallel_bubble_sort_charly (uint64_t *T, const uint64_t size){
    register unsigned int i ;
    uint64_t temp;
    int pasfini = 1;
    int pasfinisecond;
    int maxnThread = omp_get_max_threads();
    int sizechunk = size / maxnThread;
    int reste = size % maxnThread;
    // printf("nombre de thread : %d, taille des espaces : %d, reste = %d\n",maxnThread,sizechunk,reste);
    
    while(!is_sorted(T,size)){
    //while(pasfini != 0){
        //pasfini = 0;
        //pasfinisecond = 0;
        /*
        printf("-------------------debut arrangement chunk-------------------\n");
        print_array(T,size);
        */
        #pragma omp parallel 
        {
            //#pragma omp for private(temp),schedule(dynamic,sizechunk),reduction(+:pasfini)
            #pragma omp for private(temp),schedule(dynamic,sizechunk)
            for(i = 0; i < size - 1 ; i++){
                //printf("%d / %d -> %d == %d / %d ->%d\n",i,sizechunk,(i / sizechunk),i+1,sizechunk, ((i+1) / sizechunk));
                if(T[i] > T[i+1] && (i / sizechunk) == ((i+1) / sizechunk)){
                    //printf("%d avec %d    ",i,i+1);
                    //pasfini++;
                    temp = T[i+1];
                    T[i+1] = T[i];
                    T[i] = temp;
                }
            }
        }

        /*
        printf("\n");
        print_array(T,size);
        printf("--------------------fin arrangement chunk--------------------\n");
        
        printf("-------------------echange entre chunk-------------------\n");
        print_array(T,size);
        */
        #pragma omp parallel
        {
            //#pragma omp for firstprivate(temp),reduction(+:pasfinisecond)
            #pragma omp for firstprivate(temp)
            for(i = 1; i < maxnThread ; i++){
                int i1 = sizechunk * i - 1;
                int i2 = sizechunk * i;
                if(T[i1] > T[i2]){
                    // printf("%d avec %d -> %d > %d    ",i1,i2,T[i1],T[i2]);
                    //pasfinisecond++;
                    temp = T[i1];
                    T[i1] = T[i2];
                    T[i2] = temp;
                }
            }
        }
        //pasfini += pasfinisecond;
        /*
        printf("\n");
        print_array(T,size);
        printf("--------------------fin echange chunk--------------------\n");
        */

    }
    //printf("fin\n");
    return;
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
        fprintf (stderr, "bubble.run N \n") ;
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
        
        sequential_bubble_sort (X, N) ;
     
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

    printf ("\n bubble serial \t\t\t %.2lf Mcycles\n\n", (double)av/1000000) ;

  
    for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
    {
#ifdef RINIT
        init_array_random (X, N);
#else
        init_array_sequence (X, N);
#endif
        
        start = _rdtsc () ;

        parallel_bubble_sort (X, N) ;

        
     
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
    printf ("\n bubble parallel \t\t %.2lf Mcycles\n\n", (double)av/1000000) ;
  
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

    sequential_bubble_sort (Y, N) ;
    parallel_bubble_sort (Z, N) ;

    if (! are_vector_equals (Y, Z, N)) {
        fprintf(stderr, "ERROR: sorting with the sequential and the parallel algorithm does not give the same result\n") ;
        exit (-1) ;
    }


    free(X);
    free(Y);
    free(Z);
    
}
© 2021 GitHub, Inc.
Terms
Privacy
Security
Status
Docs
Contact GitHub
Pricing
API
Training
Blog
About
