#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "utils/jsonb.h"
#include "utils/elog.h"

extern Datum insert_event(PG_FUNCTION_ARGS);
extern Datum query_filters(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(test_insert_event);
PG_FUNCTION_INFO_V1(test_query_filters);

Datum test_insert_event(PG_FUNCTION_ARGS) {
    return insert_event(fcinfo);
}

Datum test_query_filters(PG_FUNCTION_ARGS) {
    return query_filters(fcinfo);
}
