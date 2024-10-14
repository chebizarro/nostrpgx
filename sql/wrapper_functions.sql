-- Wrapper functions for JSON input
CREATE OR REPLACE FUNCTION nostrpgx.insert_event_json(event_json jsonb) RETURNS void AS $$
DECLARE
    event_data nostrpgx.event;
BEGIN
    event_data := nostrpgx.json_to_event(event_json);
    PERFORM nostrpgx.insert_event(event_data);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION nostrpgx.bulk_insert_events_json(events_json jsonb) RETURNS void AS $$
DECLARE
    events_data nostrpgx.event[];
BEGIN
    events_data := nostrpgx.json_to_events(events_json);
    PERFORM nostrpgx.bulk_insert_events(events_data);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION nostrpgx.delete_event_json(event_id_json jsonb) RETURNS void AS $$
DECLARE
    event_id text;
BEGIN
    event_id := event_id_json->>'event_id';
    PERFORM nostrpgx.delete_event(event_id);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION nostrpgx.bulk_delete_events_json(event_ids_json jsonb) RETURNS void AS $$
DECLARE
    event_ids text[];
BEGIN
    event_ids := ARRAY(SELECT jsonb_array_elements_text(event_ids_json));
    PERFORM nostrpgx.bulk_delete_events(event_ids);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION nostrpgx.query_filters_json(filters_json jsonb) RETURNS SETOF jsonb AS $$
DECLARE
    filters_data nostrpgx.filter[];
BEGIN
    filters_data := nostrpgx.json_to_filters(filters_json);
    RETURN QUERY SELECT * FROM nostrpgx.query_filters(filters_data);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION nostrpgx.query_filters_count_json(filters_json jsonb) RETURNS int8 AS $$
DECLARE
    filters_data nostrpgx.filter[];
BEGIN
    filters_data := nostrpgx.json_to_filters(filters_json);
    RETURN nostrpgx.query_filters_count(filters_data);
END;
$$ LANGUAGE plpgsql;