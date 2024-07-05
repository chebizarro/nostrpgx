-- Test Case 2: Query by Event ID
SELECT * FROM query_filters('{
    "ids": ["event_id_1"]
}'::jsonb, false);

-- Test Case 3: Query by Author
SELECT * FROM query_filters('{
    "authors": ["author_1"]
}'::jsonb, false);

-- Test Case 4: Query by Kind
SELECT * FROM query_filters('{
    "kinds": [1]
}'::jsonb, false);

-- Test Case 5: Query by Timestamp Range
SELECT * FROM query_filters('{
    "since": 1625097600,
    "until": 1625098600
}'::jsonb, false);

-- Test Case 6: Query by Search Term
SELECT * FROM query_filters('{
    "search": "test event"
}'::jsonb, false);

-- Test Case 7: Query by Tag
SELECT * FROM query_filters('{
    "#e": ["5c83da77af1dec6d7289834998ad7aafbd9e2191396d75ec3cc27f5a77226f36"]
}'::jsonb, false);

-- Test Case 8: Combined Query
SELECT * FROM query_filters('{
    "authors": ["author_1"],
    "kinds": [1],
    "search": "test event",
    "#e": ["5c83da77af1dec6d7289834998ad7aafbd9e2191396d75ec3cc27f5a77226f36"]
}'::jsonb, false);

-- Test Case 9: Query Count
SELECT query_filters('{
    "authors": ["author_1"],
    "kinds": [1],
    "search": "test event"
}'::jsonb, true);

-- Test Case 10: Query the new event by its ID
SELECT * FROM query_filters('{
    "ids": ["event_id_2"]
}'::jsonb, false);
