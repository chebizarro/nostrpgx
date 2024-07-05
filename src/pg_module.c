#include "postgres.h"
#include "fmgr.h"
#include "utils/hsearch.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "utils/jsonb.h"
#include "catalog/pg_type.h"
#include "tag_validators.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

void _PG_init(void);
void _PG_fini(void);

HTAB *tag_validator_map;

void _PG_init(void) {
    HASHCTL hash_ctl;
    memset(&hash_ctl, 0, sizeof(hash_ctl));
    hash_ctl.keysize = 16;
    hash_ctl.entrysize = sizeof(TagValidatorEntry);
    tag_validator_map = hash_create("Tag Validator Map", 16, &hash_ctl, HASH_ELEM | HASH_BLOBS);

    // Load tag validators from tag_validators.c
    register_tag_validators(tag_validator_map);
}

void _PG_fini(void) {
    hash_destroy(tag_validator_map);
}
