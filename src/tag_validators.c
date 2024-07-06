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

// Register tag validators
void register_tag_validators(HTAB *tag_validator_map) {
    // Register validators for each tag
    extern void register_e_tag_validator(HTAB *tag_validator_map);
    extern void register_p_tag_validator(HTAB *tag_validator_map);

    register_e_tag_validator(tag_validator_map);
    register_p_tag_validator(tag_validator_map);

}

void validate_and_index_tag(const char *event_id, const char *tag_name, const char *tag_value, Jsonb *metadata) {
    TagValidatorEntry *entry = (TagValidatorEntry *) hash_search(tag_validator_map, tag_name, HASH_FIND, NULL);
    if (entry && entry->validator) {
        entry->validator(event_id, tag_value, metadata);
    } else {
        ereport(ERROR, (errmsg("No validator found for tag: %s", tag_name)));
    }
}
