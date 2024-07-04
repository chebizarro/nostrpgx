#include "nostr.h"
#define USE_JSON_C
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NostrFilter *parse_nostr_filter(const char *json_str) {
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(json_str);

    NostrFilter *filter = (NostrFilter *)malloc(sizeof(NostrFilter));
    memset(filter, 0, sizeof(NostrFilter));

    struct json_object *authors;
    if (json_object_object_get_ex(parsed_json, "authors", &authors)) {
        filter->num_authors = json_object_array_length(authors);
        filter->authors = (char **)malloc(filter->num_authors * sizeof(char *));
        for (int i = 0; i < filter->num_authors; i++) {
            filter->authors[i] = strdup(json_object_get_string(json_object_array_get_idx(authors, i)));
        }
    }

    struct json_object *kinds;
    if (json_object_object_get_ex(parsed_json, "kinds", &kinds)) {
        filter->num_kinds = json_object_array_length(kinds);
        filter->kinds = (int *)malloc(filter->num_kinds * sizeof(int));
        for (int i = 0; i < filter->num_kinds; i++) {
            filter->kinds[i] = json_object_get_int(json_object_array_get_idx(kinds, i));
        }
    }

    struct json_object *since;
    if (json_object_object_get_ex(parsed_json, "since", &since)) {
        filter->since = json_object_get_int64(since);
    }

    struct json_object *until;
    if (json_object_object_get_ex(parsed_json, "until", &until)) {
        filter->until = json_object_get_int64(until);
    }

    struct json_object *tags;
    if (json_object_object_get_ex(parsed_json, "tags", &tags)) {
        filter->num_tags = json_object_object_length(tags);
        filter->tags = (NostrTag *)malloc(filter->num_tags * sizeof(NostrTag));
        int tag_index = 0;
        json_object_object_foreach(tags, key, val) {
            filter->tags[tag_index].key = strdup(key);
            filter->tags[tag_index].value = strdup(json_object_get_string(val));
            tag_index++;
        }
    }

    json_object_put(parsed_json);
    return filter;
}

void free_nostr_filter(NostrFilter *filter) {
    for (int i = 0; i < filter->num_authors; i++) {
        free(filter->authors[i]);
    }
    free(filter->authors);

    free(filter->kinds);

    for (int i = 0; i < filter->num_tags; i++) {
        free(filter->tags[i].key);
        free(filter->tags[i].value);
    }
    free(filter->tags);

    free(filter);
}

void construct_sql_query(const NostrFilter *filter, char *sql_query, size_t query_size) {
    snprintf(sql_query, query_size, "SELECT * FROM events WHERE 1=1");

    if (filter->num_authors > 0) {
        strcat(sql_query, " AND author IN (");
        for (int i = 0; i < filter->num_authors; i++) {
            strcat(sql_query, "'");
            strcat(sql_query, filter->authors[i]);
            strcat(sql_query, "'");
            if (i < filter->num_authors - 1) {
                strcat(sql_query, ", ");
            }
        }
        strcat(sql_query, ")");
    }

    if (filter->num_kinds > 0) {
        strcat(sql_query, " AND kind IN (");
        for (int i = 0; i < filter->num_kinds; i++) {
            char kind_str[32];
            snprintf(kind_str, sizeof(kind_str), "%d", filter->kinds[i]);
            strcat(sql_query, kind_str);
            if (i < filter->num_kinds - 1) {
                strcat(sql_query, ", ");
            }
        }
        strcat(sql_query, ")");
    }

    if (filter->since > 0) {
        char since_str[32];
        snprintf(since_str, sizeof(since_str), "%ld", filter->since);
        strcat(sql_query, " AND created_at >= ");
        strcat(sql_query, since_str);
    }

    if (filter->until > 0) {
        char until_str[32];
        snprintf(until_str, sizeof(until_str), "%ld", filter->until);
        strcat(sql_query, " AND created_at <= ");
        strcat(sql_query, until_str);
    }

    if (filter->num_tags > 0) {
        for (int i = 0; i < filter->num_tags; i++) {
            strcat(sql_query, " AND tags @> '{\"");
            strcat(sql_query, filter->tags[i].key);
            strcat(sql_query, "\": \"");
            strcat(sql_query, filter->tags[i].value);
            strcat(sql_query, "\"}'");
        }
    }
}
