#include "nostr.h"
#include <stdio.h>
#include <stdlib.h>

static void print_filter(const NostrFilter *filter) {
    printf("Authors:\n");
    for (int i = 0; i < filter->num_authors; i++) {
        printf("  %s\n", filter->authors[i]);
    }

    printf("Kinds:\n");
    for (int i = 0; i < filter->num_kinds; i++) {
        printf("  %d\n", filter->kinds[i]);
    }

    printf("Since: %ld\n", filter->since);
    printf("Until: %ld\n", filter->until);

    printf("Tags:\n");
    for (int i = 0; i < filter->num_tags; i++) {
        printf("  %s: %s\n", filter->tags[i].key, filter->tags[i].value);
    }
}

int main() {
    const char *json_str = "{\"authors\": [\"pubkey1\", \"pubkey2\"], \"kinds\": [1, 2, 3], \"since\": 1625097600, \"until\": 1625097700, \"tags\": {\"e\": \"event1\", \"p\": \"pubkey1\"}}";
    
    NostrFilter *filter = parse_nostr_filter(json_str);
    
    if (filter) {
        char sql_query[1024] = {0};
        construct_sql_query(filter, sql_query, sizeof(sql_query));
        
        printf("SQL Query:\n%s\n", sql_query);
        
        print_filter(filter);
        
        free_nostr_filter(filter);
    } else {
        fprintf(stderr, "Failed to parse filter\n");
        return EXIT_FAILURE;
    }

    // Call test functions
    printf("Running test_insert_nostr_event...\n");
    test_insert_nostr_event();

    printf("Running test_retrieve_nostr_event...\n");
    test_retrieve_nostr_event();

    printf("Running test_nostr_event_type...\n");
    test_nostr_event_type();

    return EXIT_SUCCESS;
}
