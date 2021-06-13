#include <son.hpp>
#include <stdio.h>
#include <inttypes.h>


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
    printf("integer_value.get_integer() == %" PRId64 "\n\n", integer_value.get_integer());

    son floating_value(103.4);
    printf("floating_value.is_floating() == %s\n", bool_to_cstr(floating_value.is_floating()));
    printf("floating_value.get_floating() == %lf\n\n", floating_value.get_floating());    

    son string_value("Hello, World!");
    printf("string_value.is_string() == %s\n", bool_to_cstr(string_value.is_string()));
    printf("string_value.get_string() == %s\n\n", string_value.get_string().c_str());

    son object_value;
    object_value.push("this_is_null", nullptr);
    object_value.push("this_is_bool", false);
    object_value.push("this_is_int", 43);

    son copy_obj = object_value;

    printf("copy == original: %s\n", bool_to_cstr(copy_obj == object_value));
    
    son small_obj = { {"a" , 1}, {"b", 2}, {"c", 3} };
    son array_value = { 17, 21, true, small_obj, small_obj, "doge" };

    object_value["array"] = array_value;
    object_value["copy of itself"] = object_value;

    print_options options;
    options.print_semicolons = true;
    options.print_commas = true;
    pretty_print(object_value, options);
    printf("\n");

    printf("Finish testing.\n");
    return 0;
}


