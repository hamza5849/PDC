#include "reducer.h"
#include <string.h>

int reduce_function(WordCount *all_counts, int total, WordCount *final_counts) {
    int num_unique = 0;
    for (int i = 0; i < total; i++) {
        if (all_counts[i].word[0] == '\0') continue;
        int found = 0;
        for (int j = 0; j < num_unique; j++) {
            if (strcmp(final_counts[j].word, all_counts[i].word) == 0) {
                final_counts[j].count += all_counts[i].count;
                found = 1;
                break;
            }
        }
        if (!found && num_unique < MAX_WORDS) {
            strncpy(final_counts[num_unique].word, all_counts[i].word, MAX_WORD_LEN - 1);
            final_counts[num_unique].count = all_counts[i].count;
            num_unique++;
        }
    }
    return num_unique;
}