EXTENSION = nostr_extension
MODULE_big = nostr_extension
OBJS = src/pg_module.o src/insert_event.o src/query_filters.o src/tag_validators.o src/test_functions.o

PG_CFLAGS = -Wno-declaration-after-statement

REGRESS = insert_event query_filters

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
