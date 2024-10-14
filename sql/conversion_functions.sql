-- Convert JSON to a single event
CREATE OR REPLACE FUNCTION nostrpgx.json_to_event(event_json jsonb) RETURNS nostrpgx.event AS $$
DECLARE
    result nostrpgx.event;
BEGIN
    SELECT INTO result
        event_json->>'event_id',
        event_json->>'author',
        (event_json->>'kind')::int,
        to_timestamp((event_json->>'created_at')::double precision),
        event_json->>'content',
        event_json->'tags'
    FROM jsonb_each(event_json);
    
    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- Convert JSON array to events array
CREATE OR REPLACE FUNCTION nostrpgx.json_to_events(events_json jsonb) RETURNS nostrpgx.event[] AS $$
DECLARE
    result nostrpgx.event[];
BEGIN
    SELECT INTO result ARRAY(
        SELECT nostrpgx.json_to_event(value)
        FROM jsonb_array_elements(events_json) AS value
    );
    
    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- Convert JSON to filter
CREATE OR REPLACE FUNCTION nostrpgx.json_to_filter(filter_json jsonb) RETURNS nostrpgx.filter AS $$
DECLARE
    result nostrpgx.filter;
BEGIN
    SELECT INTO result
        ARRAY(SELECT jsonb_array_elements_text(filter_json->'ids')),
        ARRAY(SELECT jsonb_array_elements_text(filter_json->'authors')),
        ARRAY(SELECT jsonb_array_elements_text(filter_json->'kinds')::int),
        ARRAY(SELECT jsonb_array_elements_text(filter_json->'#e')),
        ARRAY(SELECT jsonb_array_elements_text(filter_json->'#p')),
        to_timestamp((filter_json->>'since')::double precision),
        to_timestamp((filter_json->>'until')::double precision),
        (filter_json->>'limit')::int,
        filter_json->>'search'
    FROM jsonb_each(filter_json);
    
    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- Convert JSON array to filters array
CREATE OR REPLACE FUNCTION nostrpgx.json_to_filters(filters_json jsonb) RETURNS nostrpgx.filter[] AS $$
DECLARE
    result nostrpgx.filter[];
BEGIN
    SELECT INTO result ARRAY(
        SELECT nostrpgx.json_to_filter(value)
        FROM jsonb_array_elements(filters_json) AS value
    );
    
    RETURN result;
END;
$$ LANGUAGE plpgsql;
