-- Create the table for storing Nostr events
CREATE TABLE nostr_events (
    id SERIAL PRIMARY KEY,
    author TEXT,
    kind INTEGER,
    created_at BIGINT,
    tags JSONB,
    content TEXT
);

-- Function to insert Nostr events
CREATE OR REPLACE FUNCTION insert_nostr_event(
    author TEXT,
    kind INTEGER,
    created_at BIGINT,
    tags JSONB,
    content TEXT
) RETURNS void
AS 'MODULE_PATHNAME', 'insert_nostr_event'
LANGUAGE C STRICT;

-- Function to insert Nostr events using custom type
CREATE OR REPLACE FUNCTION insert_nostr_event_type(
    event nostr_event
) RETURNS void
AS 'MODULE_PATHNAME', 'insert_nostr_event_type'
LANGUAGE C STRICT;

-- Create custom type for Nostr events
CREATE TYPE nostr_event AS (
    author TEXT,
    kind INTEGER,
    created_at BIGINT,
    tags JSONB,
    content TEXT
);

-- Function to search for Nostr events
CREATE OR REPLACE FUNCTION nostr_filter_search(
    filter JSONB
) RETURNS SETOF JSONB
AS 'MODULE_PATHNAME', 'nostr_filter_search'
LANGUAGE C STRICT;

-- Test function to insert a Nostr event
CREATE OR REPLACE FUNCTION test_insert_nostr_event()
RETURNS void
AS 'MODULE_PATHNAME', 'test_insert_nostr_event'
LANGUAGE C STRICT;

-- Test function to retrieve a Nostr event
CREATE OR REPLACE FUNCTION test_retrieve_nostr_event()
RETURNS void
AS 'MODULE_PATHNAME', 'test_retrieve_nostr_event'
LANGUAGE C STRICT;

-- Test function to construct SQL query from Nostr filter
CREATE OR REPLACE FUNCTION test_nostr_event_type()
RETURNS void
AS 'MODULE_PATHNAME', 'test_nostr_event_type'
LANGUAGE C STRICT;

-- Call the test functions
SELECT test_insert_nostr_event();
SELECT test_retrieve_nostr_event();
SELECT test_nostr_event_type();
