#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapper.h"
#include "reducer.h"

#define TAG_CHUNK_SIZE  10
#define TAG_CHUNK_DATA  11
#define TAG_RESULT_CNT  12
#define TAG_RESULT_DATA 13
#define INPUT_FILE  "input.txt"
#define OUTPUT_FILE "output.txt"

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start = MPI_Wtime();

    if (rank == 0) {
        int num_workers = size - 1;
        FILE *fp = fopen(INPUT_FILE, "r");
        if (!fp) {
            fprintf(stderr, "Cannot open input.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *file_data = (char *)malloc(file_size + 1);
        if (fread(file_data, 1, file_size, fp) != (size_t)file_size) {
            fprintf(stderr, "Error reading file\n");
        }
        file_data[file_size] = '\0';
        fclose(fp);

        printf("\n========================================\n");
        printf("   MPI MapReduce Word Count\n");
        printf("========================================\n");
        printf("[Master] Processes : %d (1 master + %d workers)\n", size, num_workers);
        printf("[Master] File size : %ld bytes\n", file_size);

        long chunk_size = file_size / num_workers;
        for (int i = 1; i <= num_workers; i++) {
            long start_pos = (i - 1) * chunk_size;
            long end_pos   = (i == num_workers) ? file_size : i * chunk_size;
            long len       = end_pos - start_pos;
            MPI_Send(&len, 1, MPI_LONG, i, TAG_CHUNK_SIZE, MPI_COMM_WORLD);
            MPI_Send(file_data + start_pos, len, MPI_CHAR, i, TAG_CHUNK_DATA, MPI_COMM_WORLD);
            printf("[Master] Sent chunk %d to Worker %d (%ld bytes)\n", i, i, len);
        }
        free(file_data);

        WordCount *all_counts = (WordCount *)malloc(MAX_WORDS * size * sizeof(WordCount));
        int total_entries = 0;
        for (int i = 1; i <= num_workers; i++) {
            int num_received;
            MPI_Recv(&num_received, 1, MPI_INT, i, TAG_RESULT_CNT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(all_counts + total_entries, num_received * sizeof(WordCount),
                     MPI_BYTE, i, TAG_RESULT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("[Master] Received %d entries from Worker %d\n", num_received, i);
            total_entries += num_received;
        }

        WordCount *final_counts = (WordCount *)malloc(MAX_WORDS * sizeof(WordCount));
        int unique_words = reduce_function(all_counts, total_entries, final_counts);

        FILE *out = fopen(OUTPUT_FILE, "w");
        if (out) {
            fprintf(out, "Word Count Results\n==================\n");
            for (int i = 0; i < unique_words; i++)
                fprintf(out, "%-25s : %d\n", final_counts[i].word, final_counts[i].count);
            fclose(out);
        }

        printf("\n========== RESULTS ==========\n");
        printf("Unique words  : %d\n", unique_words);
        printf("Total time    : %.4f seconds\n", MPI_Wtime() - start);
        printf("Output file   : %s\n", OUTPUT_FILE);
        printf("=============================\n\n");

        free(all_counts);
        free(final_counts);

    } else {
        long chunk_len;
        MPI_Recv(&chunk_len, 1, MPI_LONG, 0, TAG_CHUNK_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char *chunk = (char *)malloc(chunk_len + 1);
        MPI_Recv(chunk, chunk_len, MPI_CHAR, 0, TAG_CHUNK_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk[chunk_len] = '\0';

        printf("[Worker %d] Processing %ld bytes\n", rank, chunk_len);
        WordCount *local_counts = (WordCount *)malloc(MAX_WORDS * sizeof(WordCount));
        int num_words = map_function(chunk, local_counts);
        printf("[Worker %d] MAP done - %d unique words\n", rank, num_words);

        MPI_Send(&num_words, 1, MPI_INT, 0, TAG_RESULT_CNT, MPI_COMM_WORLD);
        MPI_Send(local_counts, num_words * sizeof(WordCount),
                 MPI_BYTE, 0, TAG_RESULT_DATA, MPI_COMM_WORLD);

        free(chunk);
        free(local_counts);
    }

    MPI_Finalize();
    return 0;
}