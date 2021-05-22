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

    son integer_value(42);
    printf("integer_value.is_integer() == %s\n", bool_to_cstr(integer_value.is_integer()));
    printf("integer_value.get_integer() == %ld\n\n", integer_value.get_integer());

    son floating_value(103.4);
    printf("floating_value.is_floating() == %s\n", bool_to_cstr(floating_value.is_floating()));
    printf("floating_value.get_floating() == %lf\n\n", floating_value.get_floating());    

    son string_value("Hello, World!");
    printf("string_value.is_string() == %s\n", bool_to_cstr(string_value.is_string()));
    printf("string_value.get_string() == %s\n\n", string_value.get_string().c_str());

    son object_value;
    object_value.push("this_is_null", son());
    object_value.push("this_is_bool", son(false));
    object_value.push("this_is_int", son(43));

    printf("{\n");
    for (son& v : object_value) {
        printf("  '?' = ");
        if (v.is_null()) { printf("null;\n"); }
        if (v.is_boolean()) { printf("%s;\n", bool_to_cstr(v.get_boolean())); }
        if (v.is_integer()) { printf("%ld;\n", v.get_integer()); }
        if (v.is_floating()) { printf("%lf;\n", v.get_floating()); }
        if (v.is_string()) { printf("%s;\n", v.get_string().c_str()); }
    }
    printf("}\n");

    printf("Finish testing.\n");
    return 0;
}


