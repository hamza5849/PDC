/*
 * MapReduce Word Count using MPI
 * Master-Worker Architecture
 * PDC-401 Assignment - Iqra University
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORD_LEN   64
#define MAX_WORDS    50000
#define MAX_CHUNK   500000

typedef struct {
    char word[MAX_WORD_LEN];
    int  count;
} WordCount;

static void normalize(char *dst, const char *src) {
    int j = 0;
    for (int i = 0; src[i] && j < MAX_WORD_LEN - 1; i++) {
        if (isalpha((unsigned char)src[i]))
            dst[j++] = (char)tolower((unsigned char)src[i]);
    }
    dst[j] = '\0';
}

static int find_word(WordCount *wc, int n, const char *word) {
    for (int i = 0; i < n; i++)
        if (strcmp(wc[i].word, word) == 0)
            return i;
    return -1;
}

static int map_phase(const char *text, int text_len, WordCount *local_wc, int rank) {
    char *buf = (char *)malloc(text_len + 1);
    if (!buf) { fprintf(stderr, "[Worker %d] malloc failed\n", rank); return 0; }
    memcpy(buf, text, text_len);
    buf[text_len] = '\0';

    int local_count = 0;
    char *token = strtok(buf, " \t\n\r.,!?;:\"'()[]{}/<>@#$%^&*-_=+|\\`~");
    while (token) {
        char norm[MAX_WORD_LEN];
        normalize(norm, token);
        if (strlen(norm) >= 2) {
            int idx = find_word(local_wc, local_count, norm);
            if (idx >= 0) {
                local_wc[idx].count++;
            } else if (local_count < MAX_WORDS) {
                strncpy(local_wc[local_count].word, norm, MAX_WORD_LEN - 1);
                local_wc[local_count].count = 1;
                local_count++;
            }
        }
        token = strtok(NULL, " \t\n\r.,!?;:\"'()[]{}/<>@#$%^&*-_=+|\\`~");
    }
    free(buf);
    return local_count;
}

static void reduce_phase(WordCount *global_wc, int *global_count,
                         WordCount *incoming, int inc_count) {
    for (int i = 0; i < inc_count; i++) {
        int idx = find_word(global_wc, *global_count, incoming[i].word);
        if (idx >= 0) {
            global_wc[idx].count += incoming[i].count;
        } else if (*global_count < MAX_WORDS) {
            global_wc[*global_count] = incoming[i];
            (*global_count)++;
        }
    }
}

static int cmp_desc(const void *a, const void *b) {
    return ((WordCount *)b)->count - ((WordCount *)a)->count;
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime();

    if (rank == 0) {
        int num_workers = size - 1;
        printf("\n========================================\n");
        printf("   MPI MapReduce - Word Count\n");
        printf("========================================\n");
        printf("[Master] Total processes : %d (1 master + %d workers)\n", size, num_workers);

        FILE *fp = fopen("input.txt", "r");
        if (!fp) {
            fprintf(stderr, "[Master] ERROR: cannot open input.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        rewind(fp);

        char *full_text = (char *)malloc(file_size + 1);
        if (!full_text) {
            fprintf(stderr, "[Master] malloc failed for full_text\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fread(full_text, 1, file_size, fp);
        full_text[file_size] = '\0';
        fclose(fp);

        printf("[Master] File size       : %ld bytes\n", file_size);
        printf("[Master] MAP phase       : distributing text to %d workers\n",
               num_workers > 0 ? num_workers : 1);

        if (num_workers > 0) {
            long chunk_size = file_size / num_workers;
            for (int w = 1; w <= num_workers; w++) {
                long start_idx = (w - 1) * chunk_size;
                long end_idx   = (w == num_workers) ? file_size : w * chunk_size;

                while (end_idx < file_size && !isspace((unsigned char)full_text[end_idx]))
                    end_idx++;

                int chunk_len = (int)(end_idx - start_idx);
                MPI_Send(&chunk_len, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
                MPI_Send(full_text + start_idx, chunk_len, MPI_CHAR, w, 1, MPI_COMM_WORLD);
                printf("[Master] Sent chunk %d bytes to Worker %d\n", chunk_len, w);
            }
        }

        WordCount *global_wc = (WordCount *)calloc(MAX_WORDS, sizeof(WordCount));
        int global_count = 0;

        if (num_workers == 0) {
            WordCount *local_wc = (WordCount *)calloc(MAX_WORDS, sizeof(WordCount));
            int lc = map_phase(full_text, (int)file_size, local_wc, 0);
            reduce_phase(global_wc, &global_count, local_wc, lc);
            free(local_wc);
            printf("[Master] Single-process mode: mapped %d unique words\n", global_count);
        } else {
            printf("[Master] REDUCE phase    : collecting results from workers\n");
            for (int w = 1; w <= num_workers; w++) {
                int inc_count;
                MPI_Recv(&inc_count, 1, MPI_INT, w, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                WordCount *incoming = (WordCount *)malloc(inc_count * sizeof(WordCount));
                MPI_Recv(incoming, inc_count * sizeof(WordCount),
                         MPI_BYTE, w, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("[Master] Received %d unique words from Worker %d\n", inc_count, w);
                reduce_phase(global_wc, &global_count, incoming, inc_count);
                free(incoming);
            }
        }

        qsort(global_wc, global_count, sizeof(WordCount), cmp_desc);

        double end_time = MPI_Wtime();
        double elapsed  = end_time - start_time;

        int top = (global_count < 20) ? global_count : 20;

        printf("\n========== RESULTS ==========\n");
        printf("Total Unique Words : %d\n", global_count);
        printf("Execution Time     : %.4f seconds\n", elapsed);
        printf("Processes Used     : %d\n", size);
        printf("Top %d Words:\n", top);
        for (int i = 0; i < top; i++)
            printf("  %-20s : %d\n", global_wc[i].word, global_wc[i].count);
        printf("=============================\n\n");

        FILE *out = fopen("output.txt", "w");
        if (out) {
            fprintf(out, "Total Unique Words: %d\n", global_count);
            fprintf(out, "Execution Time: %.4f seconds\n", elapsed);
            fprintf(out, "Processes Used: %d\n", size);
            fprintf(out, "Workers Used: %d\n", num_workers);
            fprintf(out, "Throughput: %.2f words/sec\n", global_count / elapsed);
            fprintf(out, "\nTop 20 Words:\n");
            for (int i = 0; i < top; i++)
                fprintf(out, "%s: %d\n", global_wc[i].word, global_wc[i].count);
            fclose(out);
        }

        free(global_wc);
        free(full_text);

    } else {
        int chunk_len;
        MPI_Recv(&chunk_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char *chunk = (char *)malloc(chunk_len + 1);
        MPI_Recv(chunk, chunk_len, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk[chunk_len] = '\0';

        WordCount *local_wc = (WordCount *)calloc(MAX_WORDS, sizeof(WordCount));
        int local_count = map_phase(chunk, chunk_len, local_wc, rank);

        printf("[Worker %d] MAP done: %d unique words from %d bytes\n",
               rank, local_count, chunk_len);

        MPI_Send(&local_count, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(local_wc, local_count * sizeof(WordCount),
                 MPI_BYTE, 0, 3, MPI_COMM_WORLD);

        free(chunk);
        free(local_wc);
    }

    MPI_Finalize();
    return 0;
}