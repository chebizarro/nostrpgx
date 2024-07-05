-- Test Case 1: Insert an event with tags
SELECT insert_event(
    '{
        "event_id": "event_id_1",
        "author": "author_1",
        "kind": 1,
        "created_at": 1625097600,
        "content": "This is a test event",
        "tags": [
            ["e", "5c83da77af1dec6d7289834998ad7aafbd9e2191396d75ec3cc27f5a77226f36"],
            ["p", "f7234bd4c1394dda46d09f35bd384dd30cc552ad5541990f98844fb06676e9ca"],
            ["a", "30023:f7234bd4c1394dda46d09f35bd384dd30cc552ad5541990f98844fb06676e9ca:abcd"],
            ["alt", "reply"]
        ]
    }'::jsonb
);

-- Insert another event with different tags
SELECT insert_event(
    '{
        "event_id": "event_id_2",
        "author": "author_2",
        "kind": 2,
        "created_at": 1625098600,
        "content": "Another test event",
        "tags": [
            ["e", "event_id_1"],
            ["p", "f7234bd4c1394dda46d09f35bd384dd30cc552ad5541990f98844fb06676e9cb"],
            ["b", "12345"]
        ]
    }'::jsonb
);
