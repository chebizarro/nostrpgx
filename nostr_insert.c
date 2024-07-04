#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "commands/trigger.h"
#include "utils/jsonb.h"
#include "nostr.h"

PG_FUNCTION_INFO_V1(insert_nostr_event);

Datum insert_nostr_event(PG_FUNCTION_ARGS) {
    char *author = text_to_cstring(PG_GETARG_TEXT_P(0));
    int kind = PG_GETARG_INT32(1);
    int64_t created_at = PG_GETARG_INT64(2);
    Jsonb *tags = PG_GETARG_JSONB_P(3);
    char *content = text_to_cstring(PG_GETARG_TEXT_P(4));
    int ret;

    char *tags_str = JsonbToCString(NULL, &tags->root, VARSIZE_ANY_EXHDR(tags));

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
