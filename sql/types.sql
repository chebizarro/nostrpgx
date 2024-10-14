-- Define the composite type for an event
CREATE TYPE event AS (
    event_id text,
    author text,
    kind int,
    created_at bigint,
    content text,
    tags tag[]
);

-- Define the composite type for a general tag
CREATE TYPE tag AS (
    tag_name text,
    tag_value text,
    metadata jsonb
);

CREATE TYPE nostrpgx.filter AS (
    ids text[],
    authors text[],
    kinds int[],
    e_tags text[],
    p_tags text[],
    since timestamp,
    until timestamp,
    limit int,
    search text
);
