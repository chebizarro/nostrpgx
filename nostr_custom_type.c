#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "commands/trigger.h"
#include "utils/jsonb.h"
#include "nostr.h"

PG_FUNCTION_INFO_V1(insert_nostr_event_type);

Datum insert_nostr_event_type(PG_FUNCTION_ARGS) {
    HeapTupleHeader rec = PG_GETARG_HEAPTUPLEHEADER(0);
    bool isnull;
    Datum d;

    d = GetAttributeByName(rec, "author", &isnull);
    char *author = isnull ? NULL : TextDatumGetCString(d);

    d = GetAttributeByName(rec, "kind", &isnull);
    int kind = isnull ? 0 : DatumGetInt32(d);

    d = GetAttributeByName(rec, "created_at", &isnull);
    int64_t created_at = isnull ? 0 : DatumGetInt64(d);

    d = GetAttributeByName(rec, "tags", &isnull);
    Jsonb *tags = isnull ? NULL : DatumGetJsonb(d);

    d = GetAttributeByName(rec, "content", &isnull);
    char *content = isnull ? NULL : TextDatumGetCString(d);

    int ret;
    char *tags_str = tags ? JsonbToCString(NULL, &tags->root, VARSIZE_ANY_EXHDR(tags)) : NULL;

    SPI_connect();
    ret = SPI_execute_with_args(
        "INSERT INTO nostr_events (author, kind, created_at, tags, content) VALUES ($1, $2, $3, $4, $5)",
        5,
        (Oid[]){TEXTOID, INT4OID, INT8OID, JSONBOID, TEXTOID},
        (Datum[]){CStringGetTextDatum(author), Int32GetDatum(kind), Int64GetDatum(created_at), CStringGetTextDatum(tags_str), CStringGetTextDatum(content)},
        NULL,
        false,
        0
    );

    if (ret != SPI_OK_INSERT) {
        elog(ERROR, "SPI_execute_with_args failed");
    }

    SPI_finish();
    PG_RETURN_VOID();
}
