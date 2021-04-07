#include "http-validator.h"
#include <ctype.h>

bool http_validator_is_space(char character) {
    return character == ' ' || character == '\t';
}

bool http_validator_is_eol(char character) {
    return character == '\r' || character == '\n';
}

bool http_validator_is_hex(char character) {
    return (character >= '0' && character <= '9') ||
           (character >= 'a' && character <= 'f') ||
           (character >= 'A' && character <= 'F');
}

bool http_validator_is_separator(char character) {
    return character == ')' ||
           character == '(' ||
           character == '<' ||
           character == '>' ||
           character == '@' ||
           character == ',' ||
           character == ';' ||
           character == ':' ||
           character == '\\' ||
           character == '"' ||
           character == '/' ||
           character == '[' ||
           character == ']' ||
           character == '?' ||
           character == '=' ||
           character == '{' ||
           character == '}' ||
           character == ' ' ||
           character == '\t';
}

bool http_validator_is_token(char character) {
    return !http_validator_is_separator(character) &&
           !iscntrl(character) &&
           character != 127;
}