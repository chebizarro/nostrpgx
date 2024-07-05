# Use the official PostgreSQL image as a base
FROM postgres:15

# Install necessary packages for building extensions
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    postgresql-server-dev-15 \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user
RUN useradd -ms /bin/bash pguser

# Set environment variables
ENV PGDATA /var/lib/postgresql/data
ENV POSTGRES_DB testdb
ENV POSTGRES_USER postgres
ENV POSTGRES_PASSWORD postgres

# Initialize the PostgreSQL data directory
RUN mkdir -p /var/lib/postgresql/data && chown -R pguser:pguser /var/lib/postgresql

# Copy the extension source code into the container
COPY . /usr/src/nostrpgx

# Set the working directory
WORKDIR /usr/src/nostrpgx

# Change ownership of the working directory to the non-root user
RUN chown -R pguser:pguser /usr/src/nostrpgx

# Switch to the non-root user
USER pguser

# Initialize the database cluster
RUN initdb -D $PGDATA

# Switch back to the root user to start PostgreSQL service
USER root

# Build the extension
RUN make && make install

# Copy the test files to the appropriate directory
#RUN su pguser -c mkdir -p /usr/share/postgresql/15/extension/sql/ && \
#    cp sql/* /usr/share/postgresql/15/extension/sql/

# Start PostgreSQL as the non-root user and run the tests
CMD ["sh", "-c", "su pguser -c 'pg_ctl -D $PGDATA start' && sleep 5 && su pguser -c 'make installcheck' && su pguser -c 'pg_ctl -D $PGDATA stop'"]
