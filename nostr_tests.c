#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "commands/extension.h"
#include "utils/jsonb.h"
#include "nostr.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(test_insert_nostr_event);

Datum test_insert_nostr_event(PG_FUNCTION_ARGS) {
    char *author = "test_author";
    int kind = 1;
    int64_t created_at = 1625097600;
    const char *tags_json = "{\"e\": \"test_event\", \"p\": \"test_pubkey\"}";
    char *content = "test_content";

    Jsonb *tags = DatumGetJsonbP(DirectFunctionCall1(jsonb_in, CStringGetDatum(tags_json)));

    SPI_connect();
    int ret = SPI_execute_with_args(
        "INSERT INTO nostr_events (author, kind, created_at, tags, content) VALUES ($1, $2, $3, $4, $5)",
        5,
        (Oid[]){TEXTOID, INT4OID, INT8OID, JSONBOID, TEXTOID},
        (Datum[]){CStringGetTextDatum(author), Int32GetDatum(kind), Int64GetDatum(created_at), JsonbPGetDatum(tags), CStringGetTextDatum(content)},
        NULL,
        false,
        0
    );

    if (ret != SPI_OK_INSERT) {
        SPI_finish();
        elog(ERROR, "SPI_execute_with_args failed");
    }

    SPI_finish();
    PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(test_retrieve_nostr_event);

Datum test_retrieve_nostr_event(PG_FUNCTION_ARGS) {
    SPI_connect();
    int ret = SPI_execute("SELECT author, kind, created_at, tags, content FROM nostr_events WHERE author = 'test_author'", true, 1);

    if (ret != SPI_OK_SELECT) {
        SPI_finish();
        elog(ERROR, "SPI_execute failed");
    }

    if (SPI_processed > 0) {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;
        SPITupleTable *tuptable = SPI_tuptable;
        HeapTuple tuple = tuptable->vals[0];
        Datum author = SPI_getbinval(tuple, tupdesc, 1, NULL);
        Datum kind = SPI_getbinval(tuple, tupdesc, 2, NULL);
        Datum created_at = SPI_getbinval(tuple, tupdesc, 3, NULL);
        Datum tags = SPI_getbinval(tuple, tupdesc, 4, NULL);
        Datum content = SPI_getbinval(tuple, tupdesc, 5, NULL);

        elog(INFO, "Author: %s", TextDatumGetCString(author));
        elog(INFO, "Kind: %d", DatumGetInt32(kind));
        elog(INFO, "Created At: %ld", DatumGetInt64(created_at));
        elog(INFO, "Tags: %s", TextDatumGetCString(DirectFunctionCall1(jsonb_out, tags)));
        elog(INFO, "Content: %s", TextDatumGetCString(content));
    } else {
        elog(INFO, "No rows found");
    }

    SPI_finish();
    PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(test_nostr_event_type);

Datum test_nostr_event_type(PG_FUNCTION_ARGS) {
    NostrFilter filter;
    filter.num_authors = 1;
    filter.authors = palloc(sizeof(char *));
    filter.authors[0] = pstrdup("test_author");
    filter.num_kinds = 1;
    filter.kinds = palloc(sizeof(int));
    filter.kinds[0] = 1;
    filter.since = 1625097600;
    filter.until = 1625097700;
    filter.num_tags = 1;
    filter.tags = palloc(sizeof(NostrTag));
    filter.tags[0].key = pstrdup("e");
    filter.tags[0].value = pstrdup("test_event");

    char sql_query[1024] = {0};
    construct_sql_query(&filter, sql_query, sizeof(sql_query));
    elog(INFO, "Constructed SQL Query: %s", sql_query);

    PG_RETURN_VOID();
}
