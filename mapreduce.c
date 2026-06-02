#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 100
#define OUTPUT_FILE "output.txt"

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start = MPI_Wtime();
    int array[ARRAY_SIZE];

    if (rank == 0) {
        int num_workers = size - 1;
        printf("\n========================================\n");
        printf("   MPI Array Sum (100 Integers)\n");
        printf("========================================\n");
        printf("[Master] Processes: %d (1 master + %d workers)\n", size, num_workers);

        // Generate the array of 100 integers
        srand(time(NULL));
        printf("[Master] Generated Array: \n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = rand() % 100; // Random numbers 0-99
            printf("%d ", array[i]);
        }
        printf("\n");

        // Data Partitioning Strategy
        int chunk_size = ARRAY_SIZE / size;
        int remainder = ARRAY_SIZE % size;

        // Send chunks to workers
        for (int i = 1; i < size; i++) {
            int start_idx = 0;
            for (int j = 0; j < i; j++) {
                start_idx += chunk_size + (j < remainder ? 1 : 0);
            }
            int local_size = chunk_size + (i < remainder ? 1 : 0);
            
            // Process Communication Model
            MPI_Send(&local_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&array[start_idx], local_size, MPI_INT, i, 1, MPI_COMM_WORLD);
            printf("[Master] Sent chunk of size %d to Worker %d\n", local_size, i);
        }

        // Master computes its own chunk
        int master_size = chunk_size + (0 < remainder ? 1 : 0);
        int local_sum = 0;
        for (int i = 0; i < master_size; i++) {
            local_sum += array[i];
        }
        printf("[Master] Computed local sum: %d\n", local_sum);

        // Result Aggregation Mechanism
        int global_sum = local_sum;
        int worker_sum;
        for (int i = 1; i < size; i++) {
            MPI_Recv(&worker_sum, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("[Master] Received partial sum %d from Worker %d\n", worker_sum, i);
            global_sum += worker_sum;
        }

        double end = MPI_Wtime();
        printf("\n========== RESULTS ==========\n");
        printf("Total Sum      : %d\n", global_sum);
        printf("Total Time     : %.4f seconds\n", end - start);
        printf("=============================\n\n");

        // Write to output.txt for the web UI
        FILE *out = fopen(OUTPUT_FILE, "w");
        if (out) {
            fprintf(out, "Total Sum: %d\n", global_sum);
            fclose(out);
        }
    } else {
        // Worker receives chunk, computes partial sum, sends back
        int local_size;
        MPI_Recv(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int *local_array = (int*)malloc(local_size * sizeof(int));
                MPI_Recv(local_array, local_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int local_sum = 0;
        for (int i = 0; i < local_size; i++) {
            local_sum += local_array[i];
        }
        
        printf("[Worker %d] Computed local sum: %d (from %d elements)\n", rank, local_sum, local_size);
        MPI_Send(&local_sum, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        
        free(local_array);
    }

    MPI_Finalize();
    return 0;
}