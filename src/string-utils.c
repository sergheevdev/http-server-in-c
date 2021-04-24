
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

StringBuilder * string_builder_create(size_t initial_capacity, size_t resize_increment) {
    if(initial_capacity < 1) {
        fprintf(stderr, "The 'initial_capacity' provided to 'string_builder_create' "
                        "must be a positive integer, bigger or equal to '1'\n");
        return NULL;
    }
    if(initial_capacity < 1) {
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
    char * next = string_builder->built_chain + stop_index;
    size_t left_to_move = string_builder->used_capacity - stop_index;
    string_builder->used_capacity -= left_to_move;
    string_builder->built_chain -= (left_to_move - 1);
    while(left_to_move != 0) {
        (* start) = (* next);
        start++;
        next++;
        left_to_move--;
    }
    return true;
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






static const int BUFFER_SIZE = 128;


/**
 * O (n+a*t) time | O (n+a*(-f+t)) space
 * - where n is the length of the original string
 * - where a is the number of "from" string ocurrences
 * - where f is the length of "from" string
 * - where t is the length of "to" string
 */
char * string_replace_all(char * original, char * from, char * to) {

    // Allocate the initial builder with its default capacity
    char * builder = malloc(sizeof(char) * BUFFER_SIZE);
    if(builder == NULL) {
        fprintf(stderr, "Unable to allocate memory for the builder at string_replace_all\n");
        return NULL;
    }

    char * builder_traverser = builder;   // Points to the next free position to append
    size_t builder_used = 0;              // Counts the amount of appended characters to the builder
    size_t builder_max = BUFFER_SIZE;     // Sets the initial maximum builder characters capacity
    char * builder_potential = builder;   // Points to the next starting potential match char position
    size_t builder_decrementor = 0;       // Counts the amount of possibly wrongly appended characters

    char * original_traverser = original; // Points to the current traversal "original" character
    char * from_traverser = from;         // Points to the character of the "from" chain that is being checked

    // While didn't finish to iterate through the original chain
    while((* original_traverser) != '\0') {

        // If the "from_traverser" is at the end of the char sequence we have a match
        if((* from_traverser) == '\0') {
            // Move the current builder traversal position to the potential start and overwrite the wrongly appended chars
            builder_traverser = builder_potential;
            builder_used -= builder_decrementor;
            // Append the "to" sequence to the builder and overwriting the previously wrongly appended chars
            char * to_traverser = to;
            while((* to_traverser) != '\0') {
                // We ensure we have space for another char and we append the "to" sequence current character
                if(builder_used == builder_max - 1) {
                    // We resize the builder to BUFFER_SIZE more characters
                    size_t new_size = builder_max + BUFFER_SIZE;
                    // Ensure the resize was successfully completed
                    char * new_builder = realloc(builder, sizeof(char) * new_size);
                    if(new_builder == NULL) {
                        fprintf(stderr, "Unable to reallocate memory of the new builder at string_replace_all\n");
                        free(builder);
                        // Prevent pointer deference
                        builder_traverser = NULL;
                        builder_potential = NULL;
                        original_traverser = NULL;
                        from_traverser = NULL;
                        return NULL;
                    }
                    // Point the builder to its new pointer and change its size to the new size
                    builder = new_builder;
                    builder_max = new_size;
                    // We update the old traversal pointer to its newly allocated one
                    builder_traverser = builder;
                    // We move the pointer to the position we were previously in
                    builder_traverser = builder_traverser + builder_used;
                }
                (* builder_traverser) = (* to_traverser); // Append the "to" char value to the builder
                builder_used++;                           // Increment the number of builder used slots
                builder_traverser++;                      // Move to the next free position for the builder
                to_traverser++;                           // Move to the next "to" sequence char to append
            }
            builder_potential = (builder_traverser);        // The next character might be a potential match
            builder_decrementor = 0;                        // Reset the missed positions amount
            from_traverser = from;                          // Restart the "from" traverser to check the next potential match from the beginning
            original_traverser--;
        }
        // If there is a character match (we continue checking if matches the whole "to" chain)
        else if((* original_traverser) == (* from_traverser)) {
            // We ensure we have space for another char and we append the char (supposing there will not be a whole match)
            if(builder_used == builder_max - 1) {
                // We resize the builder to BUFFER_SIZE more characters
                size_t new_size = builder_max + BUFFER_SIZE;
                // Ensure the resize was successfully completed
                char * new_builder = realloc(builder, sizeof(char) * new_size);
                if(new_builder == NULL) {
                    fprintf(stderr, "Unable to reallocate memory of the new builder at string_replace_all\n");
                    free(builder);
                    // Prevent pointer deference
                    builder_traverser = NULL;
                    builder_potential = NULL;
                    original_traverser = NULL;
                    from_traverser = NULL;
                    return NULL;
                }
                // Point the builder to its new pointer and change its size to the new size
                builder = new_builder;
                builder_max = new_size;
                // We update the old traversal pointer to its newly allocated one
                builder_traverser = builder;
                // We move the pointer to the position we were previously in
                builder_traverser = builder_traverser + builder_used;
            }
            (* builder_traverser) = (* original_traverser); // Append the "original" char value to the builder
            builder_used++;                                 // Increment the number of builder used slots
            builder_traverser++;                            // Move to the next free position for the builder
            builder_decrementor++;                          // If we are wrong and it is a match we store the amount of missed positions
            from_traverser++;                               // Move to next position to check if the following chars also match
        }
        // The chain does not match so we were right so we can reset the amount of missed positions and the potential match
        else {

            // We ensure we have space for another char and we append the non-matching char which is part of the string
            if(builder_used == builder_max - 1) {
                // We resize the builder to BUFFER_SIZE more characters
                size_t new_size = builder_max + BUFFER_SIZE;
                // Ensure the resize was successfully completed
                char * new_builder = realloc(builder, sizeof(char) * new_size);
                if(new_builder == NULL) {
                    fprintf(stderr, "Unable to reallocate memory of the new builder at string_replace_all\n");
                    free(builder);
                    // Prevent pointer deference
                    builder_traverser = NULL;
                    builder_potential = NULL;
                    original_traverser = NULL;
                    from_traverser = NULL;
                    return NULL;
                }
                // Point the builder to its new pointer and change its size to the new size
                builder = new_builder;
                builder_max = new_size;
                // We update the old traversal pointer to its newly allocated one
                builder_traverser = builder;
                // We move the pointer to the position we were previously in
                builder_traverser = builder_traverser + builder_used;
            }
            (* builder_traverser) = (* original_traverser); // Append the "original" char value to the builder
            builder_used++;                                 // Increment the number of builder used slots
            builder_traverser++;                            // Move to the next free position for the builder

            builder_potential = (builder_traverser);        // The next character might be a potential match
            builder_decrementor = 0;                        // Reset the missed positions amount
            from_traverser = from;                          // Restart the "from" traverser to check the next potential match from the beginning
        }
        original_traverser++;
    }
    (* builder_traverser) = '\0';
    builder_used++;
    // If the space used is bigged than needed we shorten the chain
    if(builder_max > builder_used) {
        char * new_builder = realloc(builder, sizeof(char) * builder_used);
        if(new_builder == NULL) {
            fprintf(stderr, "Unable to reallocate memory of the shortened builder at string_replace_all\n");
            free(builder);
            // Prevent pointer deference
            builder_traverser = NULL;
            builder_potential = NULL;
            original_traverser = NULL;
            from_traverser = NULL;
            return NULL;
        }
        builder = new_builder;
    }

    // Prevent pointer deference
    builder_traverser = NULL;
    builder_potential = NULL;
    original_traverser = NULL;
    from_traverser = NULL;

    // Return the built result
    return builder;
}

struct test {
    char * original;
    char * from;
    char * to;
    char * expected;
};

typedef struct test Test;

Test tests[] = {
    {
        "HeLLo darkness my oLd friend",
        "L",
        "l",
        "Hello darkness my old friend"
    },
    {
        "Hello {name}! Are you feeling good today? I hope yes, {name}!",
        "{name}",
        "Emilie",
        "Hello Emilie! Are you feeling good today? I hope yes, Emilie!"
    },
    {
        "Let's try to remove a character",
        "e",
        "",
        "Lt's try to rmov a charactr"
    },
    {
        "111222333222111",
        "2",
        "--++*++--",
        "111--++*++----++*++----++*++--333--++*++----++*++----++*++--111"
    },
    {
        "{ 'country': '{country}', 'max_ping': '35' }",
        "{country}",
        "Spain",
        "{ 'country': 'Spain', 'max_ping': '35' }"
    },
    {
        "",
        "{action}",
        "Perform",
        ""
    },
    {
        "",
        "",
        "",
        ""
    },
    {
        "aaaaa aaaaa aaaaa",
        "aa",
        "bb",
        "bbbba bbbba bbbba"
    },
    {
        "ababa ababa ababa",
        "aba",
        "X",
        "Xba Xba Xba",
    },
    {
        "{ 'restart': '%value%' }",
        "%value%",
        "true",
        "{ 'restart': 'true' }"
    }
};

int main() {

    int size = sizeof(tests) / sizeof(tests[0]);
    for(int i = 0; i < size; i++) {
        Test current_test = tests[i];
        char * original = strdup(current_test.original);
        char * from = strdup(current_test.from);
        char * to = strdup(current_test.to);
        char * result = string_replace_all(original, from, to);
        if(result != NULL) {
            if(strcmp(result, current_test.expected) != 0) {
                fprintf(stderr, "Assertion failed\n");
                printf("Given: %s but expected: %s\n", result, current_test.expected);
                exit(1);
            }
            free(result);
        } else {
            free(original);
            free(from);
            free(to);
            fprintf(stderr, "Error\n");
            exit(1);
        }
        free(original);
        free(from);
        free(to);
    }
    printf("All tests passed successfully");

}
