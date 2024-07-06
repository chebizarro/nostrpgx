DO $$
BEGIN
    IF EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'postgis') THEN
        PERFORM 1;
    ELSE
        RAISE NOTICE 'PostGIS is not installed. Geospatial types will not be available.';
    END IF;
END $$;
