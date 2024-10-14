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
#include "executor/spi.h"
#include "catalog/pg_type.h"
#include "utils/elog.h"
#include "utils/lsyscache.h"

extern char* construct_query(ArrayType *filters_array, bool is_count);

PG_FUNCTION_INFO_V1(query_filters);

Datum query_filters(PG_FUNCTION_ARGS) {
    ArrayType *filters_array = PG_GETARG_ARRAYTYPE_P(0);
    char *sql = construct_query(filters_array, false);

    if (sql == NULL) {
        PG_RETURN_NULL();
    }

    SPI_connect();
    int ret = SPI_exec(sql, 0);
    if (ret != SPI_OK_SELECT) {
        SPI_finish();
        ereport(ERROR, (errmsg("Error executing query: %s", sql)));
    }

    TupleDesc tupdesc = SPI_tuptable->tupdesc;
    int num_tuples = SPI_processed;

    JsonbParseState *state = NULL;
    JsonbValue *result = pushJsonbValue(&state, WJB_BEGIN_ARRAY, NULL);

    for (int i = 0; i < num_tuples; i++) {
        HeapTuple tuple = SPI_tuptable->vals[i];
        Datum datum = heap_getattr(tuple, 1, tupdesc, NULL);
        Jsonb *jsonb = DatumGetJsonbP(datum);
        result = pushJsonbValue(&state, WJB_ELEM, &jsonb->root);
    }

    result = pushJsonbValue(&state, WJB_END_ARRAY, NULL);
    Jsonb *jsonb_result = JsonbValueToJsonb(result);

    SPI_finish();

    PG_RETURN_JSONB_P(jsonb_result);
}
