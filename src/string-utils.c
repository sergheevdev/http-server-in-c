
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct string_builder {
    char * built_chain;
    size_t initial_capacity;
    size_t used_capacity;
    size_t max_capacity;
    size_t resize_increment;
};

typedef struct string_builder StringBuilder;


void assert(int condition, char * message) {
    if(condition != 1) {
        printf("%s\n", message);
        exit(1);
    }
}

/**
 * Creates a string builder with the default initial capacity and resize increment.
 *
 * The returned builder must be freed by the client after its usage.
 *
 * @return a new string builder
 */
StringBuilder * string_builder_create_default();

/**
 * Creates a string builder with a concrete initial capacity and resize increment.
 *
 * The returned builder must be freed by the client after its usage.
 *
 * @param initial_capacity initial amount of dynamically allocated characters
 * @param resize_increment the increment of characters on resize
 *
 * @return a new string builder
 */
StringBuilder * string_builder_create(size_t initial_capacity, size_t resize_increment);

/**
 * Frees the string builder structure and all its dynamically allocated contents.
 *
 * @param string_builder the string builder that is about to be freed
 */
void string_builder_destroy(StringBuilder * string_builder);

/**
 * Appends a character to the given string builder.
 *
 * @param string_builder the string builder to whom the character must be appended to
 * @param character the character to be appended
 *
 * @return true if the append operation completed successfully, false otherwise
 */
bool string_builder_append(StringBuilder * string_builder, char character);

/**
 * Returns the builder's internal pointer to the constructed string.
 *
 * This string is freed when you free the builder structure (use with caution).
 *
 * @param string_builder the string builder from whom the built chain is extracted
 *
 * @return the built chain by the given string builder
 */
char * string_builder_result(StringBuilder * string_builder);

/**
 * Returns a copy of the constructed string.
 *
 * The returned string must be freed by the client after its usage.
 *
 * @param string_builder the string builder from whom the built chain copy is extracted
 *
 * @return a copy of the string builder's built chain
 */
char * string_builder_result_as_copy(StringBuilder * string_builder);

// Default implementation values (to be statistically tested for best default values)
static const int DEFAULT_INITIAL_CAPACITY = 128;
static const int DEFAULT_RESIZE_INCREMENT = 64;

StringBuilder * string_builder_create_default() {
    return string_builder_create(DEFAULT_INITIAL_CAPACITY, DEFAULT_RESIZE_INCREMENT);
}

void string_builder_create_default_test() {
    StringBuilder * string_builder = string_builder_create_default();
    assert(string_builder != NULL, "Expected 'string_builder' not to be NULL");
    assert(string_builder->built_chain != NULL, "Expected 'built_chain' not to be NULL");
    assert(string_builder->initial_capacity == DEFAULT_INITIAL_CAPACITY, "Expected 'initial_capacity' to be equal to '128'");
    assert(string_builder->used_capacity == 0, "Expected 'used_capacity' to be equal to '0'");
    assert(string_builder->max_capacity == DEFAULT_INITIAL_CAPACITY, "Expected 'max_capacity' to be equal to '128'");
    assert(string_builder->resize_increment == DEFAULT_RESIZE_INCREMENT, "Expected 'resize_increment' to be equal to '64'");
    string_builder_destroy(string_builder);
    printf("The test 'string_builder_create_default_test' passed successfully!\n");
}

StringBuilder * string_builder_create(size_t initial_capacity, size_t resize_increment) {
    if(initial_capacity < 0) {
        fprintf(stderr, "The 'initial_capacity' provided to 'string_builder_create' "
                        "must be a positive integer, bigger or equal to '1'\n");
        return NULL;
    }
    if(resize_increment < 1) {
        fprintf(stderr, "The 'resize_increment' provided to 'string_builder_create' "
                        "must be a positive integer, bigger or equal to '1'\n");
        return NULL;
    }
    char * built_chain = malloc(sizeof(char) * initial_capacity);
    if(built_chain == NULL) {
        fprintf(stderr, "Unable to allocate memory for 'built_chain' at 'string_builder_create'\n");
        return NULL;
    }
    StringBuilder * string_builder = malloc(sizeof(StringBuilder));
    if(string_builder == NULL) {
        fprintf(stderr, "Unable to allocate memory for 'string_builder' at 'string_builder_create'\n");
        free(built_chain);
        return NULL;
    }
    string_builder->built_chain = built_chain;
    string_builder->initial_capacity = initial_capacity;
    string_builder->used_capacity = 0;
    string_builder->max_capacity = initial_capacity;
    string_builder->resize_increment = resize_increment;
    return string_builder;
}

void string_builder_create_using_invalid_resize_increment_test() {
    StringBuilder * string_builder = string_builder_create(10, 0);
    assert(string_builder == NULL, "Expected the builder to be NULL");
    string_builder_destroy(string_builder);
    printf("The test 'string_builder_create_using_invalid_resize_increment_test' passed successfully!\n");
}

void string_builder_destroy(StringBuilder * string_builder) {
    if(string_builder == NULL) return;
    if(string_builder->built_chain != NULL) free(string_builder->built_chain);
    free(string_builder);
}

bool string_builder_ensure_capacity(StringBuilder * string_builder, size_t chars_amount) {
    if(string_builder == NULL) {
        fprintf(stderr, "Trying to ensure the capacity of a NULL builder at 'string_builder_ensure_capacity'\n");
        return false;
    }
    if(chars_amount < 1) {
        fprintf(stderr, "The 'chars_amount' capacity to be ensured at 'string_builder_create' "
                        "must be a positive integer, bigger or equal to '1'\n");
    }
    // Always ensure one spot for the string null terminator
    while(string_builder->used_capacity + chars_amount > string_builder->max_capacity - 1) {
        // Increment the builder's size (new size = old size + increment)
        size_t new_size = string_builder->max_capacity + string_builder->resize_increment;
        char * resized_chain = realloc(string_builder->built_chain, sizeof(char) * new_size);
        if(resized_chain == NULL) {
            fprintf(stderr, "Unable to reallocate memory for 'resized_chain' at 'string_builder_ensure_capacity'\n");
            return false;
        }
        string_builder->built_chain = resized_chain;
        string_builder->max_capacity = new_size;
    }
    return true;
}

void string_builder_ensure_capacity_test() {
    StringBuilder * string_builder = string_builder_create(5, 10);
    for(int i = 0; i < 5; i++) {
        string_builder_append(string_builder, 'A');
    }
    assert(string_builder->used_capacity == 5, "Expected used capacity to be equal to '5'");
    assert(string_builder->max_capacity == 15, "Expected max capacity to be equal to '15'");
    string_builder_destroy(string_builder);
    printf("The test 'string_builder_ensure_capacity_test' passed successfully!\n");
}

bool string_builder_append(StringBuilder * string_builder, char character) {
    if(string_builder == NULL) {
        fprintf(stderr, "Trying to append a character to a NULL builder at 'string_builder_append'\n");
        return false;
    }
    // Make sure the builder's capacity allows one more character (if not resize the buffer)
    bool is_capacity_ensured = string_builder_ensure_capacity(string_builder, 1);
    if(is_capacity_ensured == false) return false;
    // Get the last unused character position to which the new character is to be appended
    char * current_position = string_builder->built_chain + string_builder->used_capacity;
    (* current_position) = character;
    string_builder->used_capacity++;
    return true;
}

void string_builder_append_test() {
    StringBuilder * string_builder = string_builder_create(2, 5);
    string_builder_append(string_builder, 'A');
    assert((* string_builder->built_chain) == 'A', "Expected first char to be equal to 'A'");
    string_builder_append(string_builder, 'B');
    assert((* (string_builder->built_chain + 1)) == 'B', "Expected second char to be equal to 'B'");
    string_builder_append(string_builder, 'C');
    assert((* (string_builder->built_chain + 2)) == 'C', "Expected third char to be equal to 'C'");
    string_builder_append(string_builder, 'D');
    assert((* (string_builder->built_chain + 3)) == 'D', "Expected fourth char to be equal to 'D'");
    string_builder_destroy(string_builder);
    printf("The test 'string_builder_append_test' passed successfully!\n");
}

bool string_builder_remove(StringBuilder * string_builder, size_t start_index, size_t stop_index) {
    if(start_index < 0) {
        fprintf(stderr, "Trying to pass an invalid 'start_index' value with "
                        "value less than '1' at 'string_builder_remove'\n");
        return false;
    }
    if(stop_index < 0) {
        fprintf(stderr, "Trying to pass an invalid 'stop_index' value with "
                        "value less than '1' at 'string_builder_remove'\n");
        return false;
    }
    if(stop_index > string_builder->max_capacity - 1) {
        fprintf(stderr, "Trying to pass an invalid 'stop_index' value with value "
                        "bigger than the chain size at 'string_builder_remove'\n");
        return false;
    }
    char * start = string_builder->built_chain + start_index;
    char * next = string_builder->built_chain + stop_index + 1;
    size_t left_to_move = string_builder->used_capacity - (stop_index + 1);
    // Adjust the capacity of the builder to the new one
    string_builder->used_capacity -= (stop_index - start_index) + 1;
    // Move the all the characters after the stop index to the left
    while(left_to_move != 0) {
        (* start) = (* next);
        start++;
        next++;
        left_to_move--;
    }
    return true;
}

void string_builder_remove_test() {
    StringBuilder * string_builder = string_builder_create(2, 5);
    string_builder_append(string_builder, 'A');
    string_builder_append(string_builder, 'B');
    string_builder_append(string_builder, 'C');
    string_builder_append(string_builder, 'D');
    string_builder_remove(string_builder, 1, 2);
    assert((* string_builder->built_chain) == 'A', "Expected first char equal to 'A'");
    assert((* (string_builder->built_chain + 1)) == 'D', "Expected second char equal to 'D'");
}

char * string_builder_result(StringBuilder * string_builder) {
    // We include the null terminator in the resize
    size_t new_size = string_builder->used_capacity + 1;
    char * resized_chain = realloc(string_builder->built_chain, sizeof(char) * new_size);
    if(resized_chain == NULL) {
        fprintf(stderr, "Unable to reallocate memory for 'resized_chain' at 'string_builder_result'\n");
        return false;
    }
    string_builder->built_chain = resized_chain;
    // Add a null terminator to our built string
    char * current_position = string_builder->built_chain + string_builder->used_capacity;
    (* current_position) = '\0';
    return string_builder->built_chain;
}

char * string_builder_result_as_copy(StringBuilder * string_builder) {
    char * copy = strdup(string_builder->built_chain);
    if(copy == NULL) {
        fprintf(stderr, "Failed to allocate memory for the 'built_chain' copy at 'string_builder_result_as_copy'\n");
        return NULL;
    }
    size_t new_size = string_builder->used_capacity + 1;
    char * resized_chain = realloc(copy, sizeof(char) * new_size);
    if(resized_chain == NULL) {
        fprintf(stderr, "Unable to reallocate memory for 'resized_chain' at 'string_builder_result'\n");
        return false;
    }
    copy = resized_chain;
    // Add a null terminator to our built string
    char * current_position = copy + string_builder->used_capacity;
    (* current_position) = '\0';
    return copy;
}






int main() {
    fclose(stderr);
    string_builder_create_default_test();
    string_builder_create_using_invalid_resize_increment_test();
    string_builder_ensure_capacity_test();
    string_builder_append_test();
    string_builder_remove_test();
}
