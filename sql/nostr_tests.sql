CREATE OR REPLACE FUNCTION test_insert_nostr_event()
RETURNS void
AS 'MODULE_PATHNAME', 'test_insert_nostr_event'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION test_retrieve_nostr_event()
RETURNS void
AS 'MODULE_PATHNAME', 'test_retrieve_nostr_event'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION test_nostr_event_type()
RETURNS void
AS 'MODULE_PATHNAME', 'test_nostr_event_type'
LANGUAGE C STRICT;

-- Call the test functions
SELECT test_insert_nostr_event();
SELECT test_retrieve_nostr_event();
SELECT test_nostr_event_type();
