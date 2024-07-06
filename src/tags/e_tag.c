/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tag_validators.h"
#include "utils/hsearch.h"
#include "utils/elog.h"

// Example validator function for "e" tag
static void validate_e_tag(const char *event_id, const char *tag_value, Jsonb *metadata) {
    if (!tag_value || strlen(tag_value) != 64) {
        ereport(ERROR, (errmsg("Invalid e tag value")));
    }
    // Additional operation: Insert into another table
    snprintf(sql, sizeof(sql), "INSERT INTO event_references (event_id, referenced_event_id) VALUES ('%s', '%s')", event_id, tag_value);
    SPI_exec(sql, 0);
}

void register_e_tag_validator(HTAB *tag_validator_map) {
    TagValidatorEntry *entry;
    bool found;

    entry = (TagValidatorEntry *) hash_search(tag_validator_map, "e", HASH_ENTER, &found);
    entry->validator = validate_e_tag;
}
