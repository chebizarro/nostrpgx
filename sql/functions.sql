-- Function to insert an event
CREATE OR REPLACE FUNCTION insert_event(e event) RETURNS void AS 'nostrpgx', 'insert_event' LANGUAGE C STRICT;

-- Function to query events by filters
CREATE OR REPLACE FUNCTION query_filters(filters jsonb) RETURNS SETOF event AS 'nostrpgx', 'query_filters' LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION get_events_json() RETURNS SETOF jsonb AS $$
BEGIN
    RETURN QUERY
    SELECT jsonb_build_object(
        'event_id', e.event_id,
        'author', e.author,
        'kind', e.kind,
        'created_at', e.created_at,
        'content', e.content,
        'tags', jsonb_agg(jsonb_build_object(
            'tag_name', t.tag_name,
            'tag_value', t.tag_value,
            'metadata', t.metadata
        ))
    ) AS event
    FROM events e
    LEFT JOIN tags t ON e.event_id = t.event_id
    GROUP BY e.event_id;
END;
$$ LANGUAGE plpgsql;
