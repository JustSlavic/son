## File format

This format is quite different from JSON despite being based on it. Here are some differences:

#### Keys are identifiers
#### Equal instead of colon
#### Semicolon instead of comma as separator in objects

    {
        name = "doge";
        species = "shibainu";
    }

#### Arrays have trailing comma

    {
        locales = [ "en", "jp", "ru", ];
    }

#### Comments are allowed

    {
        weight = 30; // kg
    }

#### Separators are optional

All separators are optional and newlines are not required to separate key-value pairs, although recommended.

    {
        name = "doge"
        weight = 30 // kg
        locales = [ "en" "jp" "ru" ]
    }

#### Top level braces (and brackets) are optional

    name = "doge"
    weight = 30 // kg
    locales = [ "en" "jp" "ru" ]

## Data structure

### Initialization

Simple data types you can initialize by providing actual value to a variable.

    son value_0; // null
    son value_1 = nullptr; // null
    son value_2 = true; // boolean
    son value_3 = 3; // integer
    son value_4 = 3.0; // floating
    son value_5 = "wow, doge"; // string

You can push into null values to make an object or an array.

    son value; // null
    value.push("key", 42); // value becomes an object

    son value; // null
    value.push(1); // value becomes an array

You can use `operator[]` to insert values into an object.

    son value;
    value["doge"] = "wow";

You can use `initializer_list` to initialize an object or an array.

    son value = { { "doge", "wow" }, { "happy", true } }; // makes an object

    son value = { 1, 2, 3, 4, 5 }; // makes an array

### Getting access to values

#### Through iterators

You can put son values into range-based for.

    son v_array = { 1, 2, 3 };
    for (auto& v : v_array) {
        pretty_print(v);
    }

To access object's contents you should call `pairs()` function.

    son v_obj = { { "doge", 1 }, { "wow", "amazing" }, { "happy", true } };
    for (auto& p : v_obj.pairs()) {
        // p is of type std::pair<std::string, son&>
    }

or you can use C++17 syntax:

    for (auto& [k, v] : v_obj.pairs()) {
        // k is of type std::string
        // v is of type son&
    }

### Printing

You can pretty-print values by calling `pretty_print()` function.

    son value = { { "doge", "wow" }, { "weight", 30 } };
    pretty_print(value);

Will produce:

    {
        doge = "wow"
        weight = 30
    }

You can pass settings object to the function, to change output stream, or the way object will look.

    FILE* file = fopen("output.son", "w");

    print_options options;
    options.output = file;
    options.print_semicolons = true;
    options.print_commas = true;
    options.indent = 4;
    options.multiline = print_options::multiline_t::enabled;

There are three options for multiline:
1. enabled - all objects and arrays will have each entry on the next line.
2. disabled - no entries of objects and arrays will be printed on the next line.
3. smart - objects with 3 key-value pairs will be printed on the single line, and more complex objects will be multilined.
