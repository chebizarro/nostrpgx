# Makefile
EXTENSION = nostrpgx
DATA = nostrpgx--1.0.sql \
		sql/check_extensions.sql \
		sql/types.sql \
		sql/functions.sql \
		sql/tags/e_tag.sql \
		sql/tags/p_tag.sql

OBJS =	src/pg_module.o \
		src/insert_event.o \
		src/query_filters.o \
		src/tag_validators.o \
		src/tags/e_tag.o \
		src/tags/p_tag.o

PG_CFLAGS = -Wno-declaration-after-statement

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

REGRESS = test_insert_event test_query_filters

# Paths to the test files
REGRESS_OPTS = --inputdir=test/sql --outputdir=test/results
