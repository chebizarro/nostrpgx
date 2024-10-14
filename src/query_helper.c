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
#include "utils/builtins.h"
#include "utils/array.h"
#include "utils/timestamp.h"
#include "fmgr.h"
#include "utils/lsyscache.h"
#include "executor/spi.h"

char* construct_query(ArrayType *filters_array, bool is_count) {
    if (ARR_NDIM(filters_array) == 0) {
        return NULL;
    }

    int num_filters = ARR_DIMS(filters_array)[0];
    Datum *filters_datums;
    bool *filters_nulls;
    int filters_count;

    deconstruct_array(filters_array, get_array_type_oid("filter", false), -1, false, 'd',
                      &filters_datums, &filters_nulls, &filters_count);

    if (filters_count == 0) {
        return NULL;
    }

    StringInfoData sql;
    initStringInfo(&sql);

    if (is_count) {
        appendStringInfo(&sql, "SELECT COUNT(*) FROM events e LEFT JOIN tags t ON e.event_id = t.event_id WHERE 1=1 ");
    } else {
        appendStringInfo(&sql, "SELECT jsonb_build_object("
                               "'event_id', e.event_id, "
                               "'author', e.author, "
                               "'kind', e.kind, "
                               "'created_at', EXTRACT(epoch FROM e.created_at)::bigint, "
                               "'content', e.content, "
                               "'tags', jsonb_agg(jsonb_build_object("
                               "'tag_name', t.tag_name, "
                               "'tag_value', t.tag_value, "
                               "'metadata', t.metadata)) "
                               ") AS event "
                               "FROM events e "
                               "LEFT JOIN tags t ON e.event_id = t.event_id "
                               "WHERE 1=1 ");
    }

    for (int i = 0; i < filters_count; i++) {
        if (filters_nulls[i]) {
            continue;
        }

        HeapTupleHeader filter_data = DatumGetHeapTupleHeader(filters_datums[i]);

        bool isnull;
        Datum ids_datum = GetAttributeByName(filter_data, "ids", &isnull);
        Datum authors_datum = GetAttributeByName(filter_data, "authors", &isnull);
        Datum kinds_datum = GetAttributeByName(filter_data, "kinds", &isnull);
        Datum e_tags_datum = GetAttributeByName(filter_data, "e_tags", &isnull);
        Datum p_tags_datum = GetAttributeByName(filter_data, "p_tags", &isnull);
        Datum since_datum = GetAttributeByName(filter_data, "since", &isnull);
        Datum until_datum = GetAttributeByName(filter_data, "until", &isnull);
        Datum limit_datum = GetAttributeByName(filter_data, "limit", &isnull);
        Datum search_datum = GetAttributeByName(filter_data, "search", &isnull);

        ArrayType *ids = DatumGetArrayTypeP(ids_datum);
        ArrayType *authors = DatumGetArrayTypeP(authors_datum);
        ArrayType *kinds = DatumGetArrayTypeP(kinds_datum);
        ArrayType *e_tags = DatumGetArrayTypeP(e_tags_datum);
        ArrayType *p_tags = DatumGetArrayTypeP(p_tags_datum);
        TimestampTz since = DatumGetTimestampTz(since_datum);
        TimestampTz until = DatumGetTimestampTz(until_datum);
        int limit = DatumGetInt32(limit_datum);
        char *search = TextDatumGetCString(search_datum);

        if (ARR_NDIM(ids) > 0) {
            appendStringInfo(&sql, " AND e.event_id = ANY (%s)", ArrayGetText(ids));
        }
        if (ARR_NDIM(authors) > 0) {
            appendStringInfo(&sql, " AND e.author = ANY (%s)", ArrayGetText(authors));
        }
        if (ARR_NDIM(kinds) > 0) {
            appendStringInfo(&sql, " AND e.kind = ANY (%s)", ArrayGetText(kinds));
        }
        if (ARR_NDIM(e_tags) > 0) {
            appendStringInfo(&sql, " AND EXISTS (SELECT 1 FROM tags t WHERE t.event_id = e.event_id AND t.tag_name = 'e' AND t.tag_value = ANY (%s))", ArrayGetText(e_tags));
        }
        if (ARR_NDIM(p_tags) > 0) {
            appendStringInfo(&sql, " AND EXISTS (SELECT 1 FROM tags t WHERE t.event_id = e.event_id AND t.tag_name = 'p' AND t.tag_value = ANY (%s))", ArrayGetText(p_tags));
        }
        if (since != 0) {
            appendStringInfo(&sql, " AND e.created_at >= '%s'", TimestampTzToCString(since));
        }
        if (until != 0) {
            appendStringInfo(&sql, " AND e.created_at <= '%s'", TimestampTzToCString(until));
        }
        if (search != NULL) {
            appendStringInfo(&sql, " AND e.content ILIKE '%%%s%%'", search);
        }
    }

    if (!is_count) {
        appendStringInfo(&sql, " GROUP BY e.event_id");
    }

    return sql.data;
}
