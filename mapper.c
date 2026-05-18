#include "mapper.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void to_lowercase(char *word) {
    for (int i = 0; word[i]; i++)
        word[i] = tolower((unsigned char)word[i]);
}

int map_function(char *chunk, WordCount *local_counts) {
    int num_words = 0;
    char *token = strtok(chunk, " \n\t\r.,!?;:\"'()-_");
    while (token != NULL && num_words < MAX_WORDS) {
        if (strlen(token) < 2) {
            token = strtok(NULL, " \n\t\r.,!?;:\"'()-_");
            continue;
        }
        to_lowercase(token);
        int found = 0;
        for (int i = 0; i < num_words; i++) {
            if (strcmp(local_counts[i].word, token) == 0) {
                local_counts[i].count++;
                found = 1;
                break;
            }
        }
        if (!found) {
            strncpy(local_counts[num_words].word, token, MAX_WORD_LEN - 1);
            local_counts[num_words].count = 1;
            num_words++;
        }
        token = strtok(NULL, " \n\t\r.,!?;:\"'()-_");
    }
    return num_words;
}