#ifndef MAPPER_H
#define MAPPER_H

#define MAX_WORDS     50000
#define MAX_WORD_LEN  100

typedef struct {
    char word[MAX_WORD_LEN];
    int  count;
} WordCount;

void to_lowercase(char *word);
int  map_function(char *chunk, WordCount *local_counts);

#endif