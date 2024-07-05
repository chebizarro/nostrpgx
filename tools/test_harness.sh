#!/bin/bash

# This script sets up the PostgreSQL environment, builds the extension, and runs the tests

# Set up PostgreSQL environment
export PGDATA=/var/lib/postgresql/data
export POSTGRES_DB=testdb
export POSTGRES_USER=postgres
export POSTGRES_PASSWORD=postgres

# Initialize the database cluster
initdb -D $PGDATA

# Start PostgreSQL
pg_ctl -D $PGDATA -l logfile start
sleep 5

# Build and install the extension
make && make install

# Run the tests
make installcheck

# Stop PostgreSQL
pg_ctl -D $PGDATA stop
