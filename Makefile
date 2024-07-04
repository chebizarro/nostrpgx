EXTENSION = nostr_filter
DATA = nostr_filter.control nostr_filter--1.0.sql nostr_events_table.sql nostr_insert.sql nostr_event_type.sql nostr_custom_type.sql
MODULE_big = nostr_filter
OBJS = nostr_filter.o nostr.o nostr_insert.o nostr_custom_type.o

# Use the VCPKG_ROOT environment variable
VCPKG_ROOT ?= $(shell echo $${VCPKG_ROOT})
VCPKG_TRIPLET = x64-linux
VCPKG_INSTALLED = $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)

# Print environment variables for debugging
$(info VCPKG_ROOT=$(VCPKG_ROOT))
$(info VCPKG_INSTALLED=$(VCPKG_INSTALLED))

CFLAGS += -I$(VCPKG_INSTALLED)/include
LDFLAGS += -L$(VCPKG_INSTALLED)/lib -ljson-c
CPPFLAGS += -I$(VCPKG_INSTALLED)/include

# Append custom flags
PG_CFLAGS = -Wno-declaration-after-statement
PG_CPPFLAGS = -I$(VCPKG_INSTALLED)/include
PG_LDFLAGS = -L$(VCPKG_INSTALLED)/lib -ljson-c 

EXTRA_CLEAN = test_nostr_filter test_nostr_filter.o nostr.o nostr_tests.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# Custom build rule for debugging
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@echo "gcc $(CPPFLAGS) $(CFLAGS) -c $< -o $@"

# Rule to build the test application
test_nostr_filter: nostr.o test_nostr_filter.o nostr_tests.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

