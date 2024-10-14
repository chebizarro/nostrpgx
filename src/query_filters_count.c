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

PG_FUNCTION_INFO_V1(query_filters_count);

Datum query_filters_count(PG_FUNCTION_ARGS) {
    ArrayType *filters_array = PG_GETARG_ARRAYTYPE_P(0);
    char *sql = construct_query(filters_array, true);

    if (sql == NULL) {
        PG_RETURN_NULL();
    }

    SPI_connect();
    int ret = SPI_exec(sql, 0);
    if (ret != SPI_OK_SELECT) {
        SPI_finish();
        ereport(ERROR, (errmsg("Error executing query: %s", sql)));
    }

    int64 count = DatumGetInt64(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, NULL));

    SPI_finish();

    PG_RETURN_INT64(count);
}
