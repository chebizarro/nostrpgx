#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "utils/jsonb.h"
#include "catalog/pg_type.h"
#include "utils/lsyscache.h"
#include "utils/elog.h"
#include "tag_validators.h"

typedef void (*tag_validator)(const char *event_id, const char *tag_value, Jsonb *metadata);

static void validate_e_tag(const char *event_id, const char *tag_value, Jsonb *metadata);
static void validate_p_tag(const char *event_id, const char *tag_value, Jsonb *metadata);

void register_tag_validators(HTAB *tag_validator_map) {
    TagValidatorEntry *entry;

    entry = hash_search(tag_validator_map, "e", HASH_ENTER, NULL);
    entry->validator = validate_e_tag;

    entry = hash_search(tag_validator_map, "p", HASH_ENTER, NULL);
    entry->validator = validate_p_tag;

    // Add more validators here
}

static void validate_e_tag(const char *event_id, const char *tag_value, Jsonb *metadata) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT 1 FROM events WHERE event_id = '%s'", tag_value);
    if (SPI_exec(sql, 0) == SPI_OK_SELECT) {
        if (SPI_processed == 0) {
            ereport(ERROR, (errmsg("Invalid event ID reference in 'e' tag: %s", tag_value)));
        }
    }

    // Additional operation: Insert into another table
    snprintf(sql, sizeof(sql), "INSERT INTO event_references (event_id, referenced_event_id) VALUES ('%s', '%s')", event_id, tag_value);
    SPI_exec(sql, 0);
}

static void validate_p_tag(const char *event_id, const char *tag_value, Jsonb *metadata) {
    if (strlen(tag_value) != 64) {
        ereport(ERROR, (errmsg("Invalid pubkey format in 'p' tag: %s", tag_value)));
    }

    // Additional operation: Create an index
    char sql[256];
    snprintf(sql, sizeof(sql), "CREATE INDEX IF NOT EXISTS idx_pubkey_%s ON events (author)", tag_value);
    SPI_exec(sql, 0);
}

void validate_default_tag(const char *event_id, const char *tag_value, Jsonb *metadata) {
    ereport(NOTICE, (errmsg("Inserting unknown tag: %s", tag_value)));
}
