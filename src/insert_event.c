/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "utils/jsonb.h"
#include "catalog/pg_type.h"
#include "utils/lsyscache.h"
#include "utils/elog.h"
#include "tag_validators.h"
#include <inttypes.h> // Include inttypes.h for PRIi64

extern HTAB *tag_validator_map;

Datum insert_event(PG_FUNCTION_ARGS);
static void validate_and_index_tag(const char *event_id, const char *tag_name, const char *tag_value, Jsonb *metadata);

Datum insert_event(PG_FUNCTION_ARGS) {
    Jsonb *event_jsonb = PG_GETARG_JSONB_P(0);
    JsonbIterator *it;
    JsonbValue v;
    JsonbIteratorToken type;

    const char *event_id = NULL;
    const char *author = NULL;
    int kind = 0;
    int64 created_at = 0;
    const char *content = NULL;
    Jsonb *tags_jsonb = NULL;

    it = JsonbIteratorInit(&event_jsonb->root);

    while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE) {
        if (type == WJB_KEY) {
            if (strcmp(v.val.string.val, "event_id") == 0) {
                JsonbIteratorNext(&it, &v, false);
                event_id = v.val.string.val;
            } else if (strcmp(v.val.string.val, "author") == 0) {
                JsonbIteratorNext(&it, &v, false);
                author = v.val.string.val;
            } else if (strcmp(v.val.string.val, "kind") == 0) {
                JsonbIteratorNext(&it, &v, false);
                kind = DatumGetInt32(DirectFunctionCall1(int4in, CStringGetDatum(v.val.string.val)));
            } else if (strcmp(v.val.string.val, "created_at") == 0) {
                JsonbIteratorNext(&it, &v, false);
                created_at = DatumGetInt64(DirectFunctionCall1(int8in, CStringGetDatum(v.val.string.val)));
            } else if (strcmp(v.val.string.val, "content") == 0) {
                JsonbIteratorNext(&it, &v, false);
                content = v.val.string.val;
            } else if (strcmp(v.val.string.val, "tags") == 0) {
                JsonbIteratorNext(&it, &v, false);
                tags_jsonb = JsonbValueToJsonb(&v);
            }
        }
    }

    if (event_id == NULL || author == NULL || content == NULL || tags_jsonb == NULL) {
        ereport(ERROR, (errmsg("Invalid event JSON")));
    }

    SPI_connect();

    char insert_event_sql[1024];
    snprintf(insert_event_sql, sizeof(insert_event_sql),
             "INSERT INTO events (event_id, author, kind, created_at, content) "
             "VALUES ('%s', '%s', %d, %" PRIi64 ", '%s')",
             event_id, author, kind, created_at, content);

    SPI_exec(insert_event_sql, 0);

    JsonbIterator *tags_it = JsonbIteratorInit(&tags_jsonb->root);
    JsonbValue tag;
    while ((type = JsonbIteratorNext(&tags_it, &tag, false)) != WJB_DONE) {
        if (type == WJB_ELEM) {
            if (tag.type == jbvArray) {
                const char *tag_name = NULL;
                const char *tag_value = NULL;
                JsonbParseState *state = NULL;
                JsonbValue *metadata = NULL;

                JsonbIterator *tag_it = JsonbIteratorInit(tag.val.binary.data);
                JsonbValue tag_elem;
                int tag_elem_index = 0;

                while ((type = JsonbIteratorNext(&tag_it, &tag_elem, false)) != WJB_DONE) {
                    if (tag_elem.type == jbvString) {
                        if (tag_elem_index == 0) {
                            tag_name = tag_elem.val.string.val;
                        } else if (tag_elem_index == 1) {
                            tag_value = tag_elem.val.string.val;
                        } else {
                            if (metadata == NULL) {
                                metadata = pushJsonbValue(&state, WJB_BEGIN_ARRAY, NULL);
                            }
                            metadata = pushJsonbValue(&state, WJB_ELEM, &tag_elem);
                        }
                        tag_elem_index++;
                    }
                }

                if (metadata != NULL) {
                    metadata = pushJsonbValue(&state, WJB_END_ARRAY, NULL);
                }

                if (tag_name == NULL || tag_value == NULL) {
                    ereport(ERROR, (errmsg("Invalid tag format")));
                }

                Jsonb *metadata_jsonb = metadata != NULL ? JsonbValueToJsonb(metadata) : NULL;

                validate_and_index_tag(event_id, tag_name, tag_value, metadata_jsonb);

                char *metadata_cstring = metadata_jsonb != NULL ? JsonbToCString(NULL, &metadata_jsonb->root, VARSIZE(metadata_jsonb)) : "";

                char insert_tag_sql[1024];
                snprintf(insert_tag_sql, sizeof(insert_tag_sql),
                         "INSERT INTO tags (event_id, tag_name, tag_value, metadata) "
                         "VALUES ('%s', '%s', '%s', '%s')",
                         event_id, tag_name, tag_value, metadata_cstring);

                SPI_exec(insert_tag_sql, 0);

                if (metadata_jsonb != NULL) {
                    pfree(metadata_cstring);
                }
            }
        }
    }

    SPI_finish();

    PG_RETURN_VOID();
}

static void validate_and_index_tag(const char *event_id, const char *tag_name, const char *tag_value, Jsonb *metadata) {
    TagValidatorEntry *entry = hash_search(tag_validator_map, tag_name, HASH_FIND, NULL);
    if (entry) {
        entry->validator(event_id, tag_value, metadata);
    } else {
        validate_default_tag(event_id, tag_value, metadata);
    }
}
