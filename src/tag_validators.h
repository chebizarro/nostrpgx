#ifndef TAG_VALIDATORS_H
#define TAG_VALIDATORS_H

#include "utils/hsearch.h"

typedef void (*tag_validator)(const char *event_id, const char *tag_value, Jsonb *metadata);

typedef struct {
    char tag_name[16];
    tag_validator validator;
} TagValidatorEntry;

void register_tag_validators(HTAB *tag_validator_map);
void validate_default_tag(const char *event_id, const char *tag_value, Jsonb *metadata);


#endif // TAG_VALIDATORS_H
