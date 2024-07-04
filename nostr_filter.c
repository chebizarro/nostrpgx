#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/jsonb.h"
#include "executor/spi.h"
#include "commands/extension.h"
#include "funcapi.h"
#include "nostr.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(nostr_filter_search);

Datum nostr_filter_search(PG_FUNCTION_ARGS) {
    Jsonb *filter = PG_GETARG_JSONB_P(0);
    char *filter_str;
    char sql_query[1024] = {0};
    int ret;
    FuncCallContext *funcctx;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        filter_str = JsonbToCString(NULL, &filter->root, VARSIZE_ANY_EXHDR(filter));
        NostrFilter *nostr_filter = parse_nostr_filter(filter_str);
        construct_sql_query(nostr_filter, sql_query, sizeof(sql_query));

        ret = SPI_connect();
        if (ret != SPI_OK_CONNECT) {
            elog(ERROR, "SPI_connect failed");
        }

        ret = SPI_execute(sql_query, true, 0);
        if (ret != SPI_OK_SELECT) {
            elog(ERROR, "SPI_execute failed");
        }

        funcctx->max_calls = SPI_processed;
        funcctx->user_fctx = SPI_tuptable;
        tupdesc = CreateTupleDescCopy(SPI_tuptable->tupdesc);
        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;

        free_nostr_filter(nostr_filter);
        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();

    if (funcctx->call_cntr < funcctx->max_calls) {
        HeapTuple tuple;
        SPITupleTable *tuptable = (SPITupleTable *) funcctx->user_fctx;
        TupleDesc tupdesc = tuptable->tupdesc;
        Datum result;

        tuple = tuptable->vals[funcctx->call_cntr];
        result = HeapTupleGetDatum(tuple);
        SRF_RETURN_NEXT(funcctx, result);
    } else {
        SPI_finish();
        SRF_RETURN_DONE(funcctx);
    }
}
