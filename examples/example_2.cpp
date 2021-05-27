#include <son.hpp>
#include <stdio.h>


const char* bool_to_cstr(bool b) {
    return b ? "true" : "false";
}


int main() {
    using namespace jslavic;

    printf("Test parsing son object from example_2.son:\n\n");

    parser p("example_2.son");
    son result = p.parse();

    pretty_print(result);
    printf("\n");

    printf("Finish testing.\n");
    return 0;
}


