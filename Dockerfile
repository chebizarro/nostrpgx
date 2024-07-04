# Use the official PostgreSQL image from the Docker Hub
FROM postgres:latest

# Install necessary build tools and dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    postgresql-server-dev-all \
    cmake \
    git \
    curl

# Install vcpkg for managing dependencies
RUN git clone https://github.com/Microsoft/vcpkg.git /vcpkg && \
    /vcpkg/bootstrap-vcpkg.sh && \
    /vcpkg/vcpkg integrate install && \
    /vcpkg/vcpkg install json-c

# Set environment variables for vcpkg
ENV VCPKG_ROOT=/vcpkg
ENV VCPKG_TRIPLET=x64-linux
ENV VCPKG_INSTALLED=$VCPKG_ROOT/installed/$VCPKG_TRIPLET

# Copy the extension source code to the container
COPY . /usr/src/nostrpgx
WORKDIR /usr/src/nostrpgx

# Build the extension
RUN make

# Add the extension to the PostgreSQL configuration
RUN echo "shared_preload_libraries = 'nostrpgx'" >> /usr/share/postgresql/postgresql.conf.sample

# Expose the default PostgreSQL port
EXPOSE 5432

# Start PostgreSQL server
CMD ["postgres"]
