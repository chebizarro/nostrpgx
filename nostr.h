#ifndef NOSTR_H
#define NOSTR_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *key;
    char *value;
} NostrTag;

typedef struct {
    char **authors;
    int *kinds;
    int64_t since;
    int64_t until;
    NostrTag *tags;
    int num_authors;
    int num_kinds;
    int num_tags;
} NostrFilter;

NostrFilter *parse_nostr_filter(const char *json_str);
void free_nostr_filter(NostrFilter *filter);
void construct_sql_query(const NostrFilter *filter, char *sql_query, size_t query_size);

#endif // NOSTR_H
