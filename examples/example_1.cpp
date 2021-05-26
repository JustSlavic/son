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
    printf("integer_value.get_integer() == %lld\n\n", integer_value.get_integer());

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

    print_options poptions;
    poptions.print_semicolons = true;
    poptions.print_commas = true;

    pretty_print(object_value, poptions);
    printf("\n");

    object_value["this_is_null"] = "DOGE!!!";
    object_value["THIS IS NEW"] = "new string";
    object_value["this_is_int"].clear();

    pretty_print(object_value, poptions);
    printf("\n");

    son copy_obj = object_value;

    printf("copy == original: %s\n", bool_to_cstr(copy_obj == object_value));

    object_value["copy of itself"] = copy_obj;
    pretty_print(object_value, poptions);
    printf("\n");

    son array_value;
    array_value.push(17);
    array_value.push(21);
    array_value.push(true);
    array_value.push(nullptr);
    array_value.push("doge");

    pretty_print(array_value, poptions);
    printf("\n");

    printf("Finish testing.\n");
    return 0;
}


