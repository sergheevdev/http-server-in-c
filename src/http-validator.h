#include <stdbool.h>

/* http-validator.h */
#ifndef HTTP_VALIDATOR_H
#define HTTP_VALIDATOR_H

bool http_validator_is_space(char character);
bool http_validator_is_eol(char character);
bool http_validator_is_hex(char character);
bool http_validator_is_separator(char character);
bool http_validator_is_token(char character);
bool http_validator_is_uri(char character);

#endif /* HTTP_VALIDATOR_H */