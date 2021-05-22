#include <son.hpp>
#include <stdio.h>


const char* bool_to_cstr(bool b) {
    return b ? "true" : "false";
}


int main() {
    using namespace jslavic;

    printf("Test is_* and get_* methods:\n\n");

    son null_value;
    printf("null_value.is_null() == %s\n\n", bool_to_cstr(null_value.is_null()));

    son boolean_value(true);
    printf("boolean_value.is_boolean() == %s\n", bool_to_cstr(boolean_value.is_boolean()));
    printf("boolean_value.get_boolean() == %s\n\n", bool_to_cstr(boolean_value.get_boolean()));

    printf("Finish testing.\n");
    return 0;
}


