CREATE OR REPLACE FUNCTION insert_nostr_event(
    author TEXT,
    kind INTEGER,
    created_at BIGINT,
    tags JSONB,
    content TEXT
) RETURNS void
AS 'MODULE_PATHNAME', 'insert_nostr_event'
LANGUAGE C STRICT;
