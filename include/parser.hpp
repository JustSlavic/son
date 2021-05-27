#ifndef SON_PARSER_HPP
#define SON_PARSER_HPP


#include <string>
#include "value.hpp"


namespace jslavic {


class parser {
public:
    struct settings_t {
        bool require_semicolons = false;
        bool require_commas = false;
    };

private:
    std::string filename;

public:
    parser(const char* filename) : filename(filename) {}
    parser(std::string filename) : filename(std::move(filename)) {}

    son parse();
};


inline son parse(std::string filename) {
    parser parser(std::move(filename));
    return parser.parse();
}


} // jslavic


#endif // SON_PARSER_HPP
