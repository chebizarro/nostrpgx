-- Create events table
CREATE TABLE events (
    id SERIAL PRIMARY KEY,
    event_id TEXT UNIQUE NOT NULL,
    author TEXT,
    kind INTEGER,
    created_at BIGINT,
    content TEXT
);

-- Create a full-text search index on the content field
CREATE INDEX idx_content_search ON events USING GIN (to_tsvector('english', content));

-- Create tags table
CREATE TABLE tags (
    id SERIAL PRIMARY KEY,
    event_id TEXT REFERENCES events(event_id) ON DELETE CASCADE,
    tag_name TEXT NOT NULL,
    tag_value TEXT NOT NULL,
    metadata JSONB DEFAULT '{}',
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Indexes for efficient querying
CREATE INDEX idx_tag_name ON tags(tag_name);
CREATE INDEX idx_tag_value ON tags(tag_value);
CREATE INDEX idx_event_id ON tags(event_id);
