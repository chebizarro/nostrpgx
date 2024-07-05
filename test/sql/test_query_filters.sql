-- Load pgTAP
CREATE EXTENSION pgtap;

-- Test Case: Query by Event ID
BEGIN;
SELECT tap.plan(1);

INSERT INTO events (event_id, author, kind, created_at, content) VALUES
('event_id_1', 'author_1', 1, 1625097600, 'This is a test event');

SELECT tap.is(
    (SELECT event_id FROM test_query_filters('{
        "ids": ["event_id_1"]
    }'::jsonb, false)),
    'event_id_1',
    'Query by event ID'
);

ROLLBACK;
