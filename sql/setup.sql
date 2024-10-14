-- Create a dedicated schema for the extension
CREATE SCHEMA IF NOT EXISTS nostrpgx;

-- Set the search path to include the extension schema
SET search_path TO nostrpgx, public;
