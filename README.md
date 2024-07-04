# Nostr Filter Extension for PostgreSQL

This PostgreSQL extension provides functionality for working with Nostr events. It includes functions for inserting Nostr events, searching for Nostr events based on a filter, and defining custom types for Nostr events.

## Features

- **Insert Nostr Events**: Insert Nostr events into a dedicated table.
- **Search Nostr Events**: Search for Nostr events using a JSONB filter.
- **Custom Type for Nostr Events**: Define a custom PostgreSQL type for Nostr events.
- **Test Functions**: Includes test functions for inserting, retrieving, and constructing SQL queries for Nostr events.

## Installation

### From Source

1. Ensure you have `pg_config` available and PostgreSQL development headers installed.
2. Install dependencies using `vcpkg`:

    ```sh
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    ./vcpkg install json-c
    export VCPKG_ROOT=$(pwd)
    export VCPKG_TRIPLET=x64-linux  # Adjust as necessary
    ```

3. Clone this repository and navigate to the directory:

    ```sh
    git clone https://github.com/chebizarro/nostrpgx.git
    cd nostrpgx
    ```

4. Build the extension:

    ```sh
    make
    ```

5. Install the extension into your PostgreSQL database:

    ```sh
    sudo make install
    ```

6. Create the extension in your database:

    ```sh
    psql -d your_database -c "CREATE EXTENSION nostr_filter"
    ```

### Using Docker

You can build and run the PostgreSQL container with the Nostr Filter extension using Docker.

#### Step-by-Step Guide

1. **Ensure you have Docker installed**.

2. **Clone this repository and navigate to the directory**:
    ```sh
    git clone https://github.com/chbizarro/nostrpgx.git
    cd nostrpgx
    ```

3. **Build the Docker image**:
    ```sh
    docker build -t nostrpgx -f Dockerfile.postgres .
    ```

4. **Run the Docker container**:
    ```sh
    docker run --name nostrpgx -e POSTGRES_PASSWORD=example -d nostrpgx
    ```

5. **Access the PostgreSQL container**:
    ```sh
    docker exec -it nostrpgx psql -U postgres
    ```

6. **Create the extension in your database**:
    ```sql
    CREATE EXTENSION nostr_filter;
    ```


## Usage

### Inserting Nostr Events

You can insert Nostr events into the `nostr_events` table using the `insert_nostr_event` function:

```sql
SELECT insert_nostr_event(
    'author', 
    1, 
    1625097600, 
    '{"e": "event1", "p": "pubkey1"}'::jsonb, 
    'content'
);
```

### Using the Custom Type for Nostr Events

You can define a custom PostgreSQL type for Nostr events and insert using the `insert_nostr_event_type` function:

```sql
SELECT insert_nostr_event_type(
    ROW('author', 1, 1625097600, '{"e": "event1", "p": "pubkey1"}'::jsonb, 'content')::nostr_event
);
```

### Searching Nostr Events

You can search for Nostr events using the `nostr_filter_search` function with a JSONB filter:

```sql
SELECT * FROM nostr_filter_search(
    '{"authors": ["pubkey1"], "kinds": [1], "since": 1625097600, "until": 1625097700, "tags": {"e": "event1"}}'::jsonb
);
```

### Running Tests

The extension includes test functions to verify the functionality:

```sql
SELECT test_insert_nostr_event();
SELECT test_retrieve_nostr_event();
SELECT test_nostr_event_type();
```

## Development

### Directory Structure

```
nostr_filter_extension/
├── Makefile
├── nostr.c
├── nostr.h
├── nostr_filter.c
├── nostr_filter.control
├── nostr_filter--1.0.sql
├── nostr_insert.c
├── nostr_custom_type.c
├── nostr_tests.c
├── test_nostr_filter.c
└── json_c_aliased.h
```

### Building and Cleaning

- To build the extension and test application:

    ```sh
    make
    ```

- To clean the build artifacts:

    ```sh
    make clean
    ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
