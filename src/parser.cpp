#include <parser.hpp>
#include <deque>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <inttypes.h>


namespace jslavic {


enum kind_t {
    TOKEN_UNDEFINED = 0,

    TOKEN_EQUAL_SIGN = '=',
    TOKEN_SEMICOLON = ';',
    TOKEN_COMMA = ',',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_KW_NULL = 256,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,

    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOATING,
    TOKEN_STRING,

    TOKEN_DOUBLE_SLASH,

    TOKEN_EOF,
};


// static const char* to_debug_string(kind_t k) {
//     switch (k) {
//         case TOKEN_UNDEFINED:     return "TOKEN_UNDEFINED";

//         case TOKEN_EQUAL_SIGN:    return "TOKEN_EQUAL_SIGN";
//         case TOKEN_SEMICOLON:     return "TOKEN_SEMICOLON";
//         case TOKEN_COMMA:         return "TOKEN_COMMA";

//         case TOKEN_BRACE_OPEN:    return "TOKEN_BRACE_OPEN";
//         case TOKEN_BRACE_CLOSE:   return "TOKEN_BRACE_CLOSE";

//         case TOKEN_PAREN_OPEN:    return "TOKEN_PAREN_OPEN";
//         case TOKEN_PAREN_CLOSE:   return "TOKEN_PAREN_CLOSE";

//         case TOKEN_BRACKET_OPEN:  return "TOKEN_BRACKET_OPEN";
//         case TOKEN_BRACKET_CLOSE: return "TOKEN_BRACKET_CLOSE";

//         case TOKEN_KW_NULL:       return "TOKEN_KW_NULL";
//         case TOKEN_KW_TRUE:       return "TOKEN_KW_TRUE";
//         case TOKEN_KW_FALSE:      return "TOKEN_KW_FALSE";

//         case TOKEN_IDENTIFIER:    return "TOKEN_IDENTIFIER";
//         case TOKEN_INTEGER:       return "TOKEN_INTEGER";
//         case TOKEN_FLOATING:      return "TOKEN_FLOATING";
//         case TOKEN_STRING:        return "TOKEN_STRING";

//         case TOKEN_DOUBLE_SLASH:  return "TOKEN_DOUBLE_SLASH";

//         case TOKEN_EOF:           return "TOKEN_EOF";
//     }

//     return "ERROR";
// }


// static const char* to_string(kind_t k) {
//     switch (k) {
//         case TOKEN_UNDEFINED:     return "? undefined";

//         case TOKEN_EQUAL_SIGN:    return "=";
//         case TOKEN_SEMICOLON:     return ";";
//         case TOKEN_COMMA:         return ",";

//         case TOKEN_BRACE_OPEN:    return "{";
//         case TOKEN_BRACE_CLOSE:   return "}";

//         case TOKEN_PAREN_OPEN:    return "(";
//         case TOKEN_PAREN_CLOSE:   return ")";

//         case TOKEN_BRACKET_OPEN:  return "[";
//         case TOKEN_BRACKET_CLOSE: return "]";

//         case TOKEN_KW_NULL:       return "null";
//         case TOKEN_KW_TRUE:       return "true";
//         case TOKEN_KW_FALSE:      return "false";

//         case TOKEN_IDENTIFIER:    return "identifier";
//         case TOKEN_INTEGER:       return "integer";
//         case TOKEN_FLOATING:      return "floating";
//         case TOKEN_STRING:        return "string";

//         case TOKEN_DOUBLE_SLASH:  return "//";

//         case TOKEN_EOF:           return "EOF";
//     }

//     return "ERROR";
// }


struct span {
    const char* begin = nullptr;
    size_t size = 0;

    span() : begin(nullptr), size(0) {}
    span(const char* begin, size_t size) : begin(begin), size(size) {}

    operator bool () { return size > 0; }
};


struct token {
    union value_t {
        int64_t integer;
        double floating;
    };

    value_t value;
    kind_t kind;

    // meta information
    size_t line_number;
    size_t char_number;
    span in_text;
    span line;
};


// void print_token(token t) {
//     printf("%3" PRIu64 ":%2" PRIu64 " token { kind = %20s; value = ", t.line_number, t.char_number, to_debug_string(t.kind));

//     switch (t.kind) {
//         case TOKEN_UNDEFINED: printf("ERROR! }\n"); break;
//         case TOKEN_EOF:       printf("EOF; }\n"); break;
//         case TOKEN_KW_NULL:   printf("null; }\n"); break;
//         case TOKEN_KW_TRUE:   printf("true; }\n"); break;
//         case TOKEN_KW_FALSE:  printf("false; }\n"); break;
//         case TOKEN_INTEGER:   printf(PRId64"; }\n", t.value.integer); break;
//         case TOKEN_FLOATING:  printf("%lf; }\n", t.value.floating); break;

//         case TOKEN_IDENTIFIER:
//         case TOKEN_STRING:
//             printf("%.*s; }\n", int(t.in_text.size), t.in_text.begin);
//             break;

//         case TOKEN_EQUAL_SIGN:
//         case TOKEN_SEMICOLON:
//         case TOKEN_COMMA:
//         case TOKEN_BRACE_OPEN:
//         case TOKEN_BRACE_CLOSE:
//         case TOKEN_PAREN_OPEN:
//         case TOKEN_PAREN_CLOSE:
//         case TOKEN_BRACKET_OPEN:
//         case TOKEN_BRACKET_CLOSE:
//             printf("'%c'; }\n", char(t.kind));
//             break;

//         case TOKEN_DOUBLE_SLASH:
//             printf("//; }\n");
//             break;
//         default:
//             printf("??? }\n");
//     }
// }



typedef bool(*predicate_t)(char);

// predicates
static inline bool is_alpha (char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static inline bool is_digit (char c ) { return c >= '0' && c <= '9'; }
static inline bool is_newline (char c) { return c == '\n' || c == '\r'; }
static inline bool is_space (char c) { return c == ' ' || c == '\t' || is_newline(c); }
static inline bool is_valid_identifier_head (char c) { return is_alpha(c) || c == '_'; }
static inline bool is_valid_identifier_body (char c) { return is_digit(c) || is_alpha(c) || c == '_'; }

// static strings 50 characters each
// static const char* spaces = "                                                                         ";
// static const char* carets = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";


struct lexer {
    const char* filename = nullptr;
    span text;
    std::deque<token> token_stream;
    std::unordered_map<std::string, kind_t> keywords;

    struct state_t {
        const char* current_line = nullptr;
        const char* current_char = nullptr;

        size_t char_counter = 1;
        size_t line_counter = 1;
    };

    state_t state;

public:
    lexer() {
        keywords.emplace("null", TOKEN_KW_NULL);
        keywords.emplace("true", TOKEN_KW_TRUE);
        keywords.emplace("false", TOKEN_KW_FALSE);
    }

    state_t get_checkpoint() const { return state; }
    void restore_checkpoint(state_t checkpoint) { state = checkpoint; }

    inline char get_char () {
        assert(state.current_char);

        if (state.current_char == text.begin + text.size) return 0;

        return *state.current_char;
    }

    inline char eat_char () {
        char c = get_char();

        if (is_newline(c)) {
            state.char_counter = 0;
            state.line_counter++;
            state.current_line = state.current_char + 1;
        }

        c = get_char();

        state.char_counter++;
        state.current_char++;

        return c;
    }

    span eat_while(predicate_t predicate) {
        auto checkpoint = get_checkpoint();
        size_t count = 0;

        char c;
        while (true) {
            c = get_char();

            if (c == 0 || not predicate(c)) { break; }

            eat_char();
            count += 1;
        }

        return span(checkpoint.current_char, count);
    }

    span eat_until(predicate_t predicate) {
        auto checkpoint = get_checkpoint();
        size_t count = 0;

        char c;
        while (true) {
            c = get_char();

            if (c == 0 || predicate(c)) { break; }

            eat_char();
            count += 1;
        }

        return span(checkpoint.current_char, count);
    }

    span eat_string(const char* str, size_t n) {
        auto checkpoint = get_checkpoint();
        size_t count = 0;

        while (count < n) {
            char c = get_char();

            if ((c == 0) || (c != str[count])) {
                restore_checkpoint(checkpoint);
                return span();
            }

            eat_char();
            count += 1;
        }

        return span(checkpoint.current_char, count);
    }

    span eat_string(const char* str) {
        auto checkpoint = get_checkpoint();
        size_t count = 0;

        while (str[count]) {
            char c = get_char();

            if ((c == 0) || (c != str[count])) {
                restore_checkpoint(checkpoint);
                return span();
            }

            eat_char();
            count += 1;
        }

        return span(checkpoint.current_char, count);
    }

    bool tokenize() {
        char c;
        while (eat_while(is_space), (c = get_char()) != 0) {
            if (c == '{' ||
                c == '}' ||
                c == '(' ||
                c == ')' ||
                c == '[' ||
                c == ']' ||
                c == '=' ||
                c == ';' ||
                c == ',')
            {
                token t;
                t.in_text.begin = state.current_char;
                t.in_text.size = 1;
                t.line_number = state.line_counter;
                t.char_number = state.char_counter;
                t.kind = kind_t(c);
                t.value.integer = 0;

                eat_char();

                token_stream.push_back(t);
                continue;
            }
            else if (c == '/') {
                span result = eat_string("//");
                if (result) {
                    eat_until(is_newline);
                    eat_while(is_newline);
                    continue;
                }
            }
            else if (c == '\"') {
                bool successful = eat_quoted_string();
                if (not successful) return false;
            }
            else if (is_digit(c) || (c == '.') || (c == '+') || (c == '-')) { // Read number, integer or float is unknown.
                bool successful = eat_number();
                if (not successful) return false;
            }
            else if (is_valid_identifier_head(c)) {
                bool successful = eat_keyword_or_identifier();
                if (not successful) return false;
            }
            else {
                auto checkpoint = get_checkpoint();
                auto result = eat_until(is_space);

                printf("%s:%lu:%lu: error: unknown lexeme ’%.*s’\n", filename, checkpoint.line_counter, checkpoint.char_counter, (int)result.size, result.begin);

                // auto line = get_line();
                // printf("   %lu | %.*s\n", checkpoint.line_counter, (int)line.length, line.start);
                // printf("   %.*s | %.*s%.*s\n",
                //     (int)(digits_in_number(checkpoint.line_counter)), spaces,
                //     (int)(checkpoint.current - line.start), spaces,
                //     (int)result.length, carets);

                // printf("Parse Error! Unknown lexeme: %.*s\n", i32(result.length), result.start);
                return false;
            }
        }

        if (c == '\0') {
            token t;
            t.in_text.begin = state.current_char;
            t.in_text.size = 1;
            t.line_number = state.line_counter;
            t.char_number = state.char_counter;
            t.kind = TOKEN_EOF;
            t.value.integer = 0;

            token_stream.push_back(t);
            return true;
        }

        return false;
    }

    // @Fix escaped newlines should not show up in resulted string.
    bool eat_quoted_string () {
        auto checkpoint = get_checkpoint();
        uint64_t length = 0;

        char c = get_char();
        if (c != '"') {
            // No characters are eated yet. Return as normal.
            return false;
        }

        eat_char(); // Skip double quote.
        length += 1;

        while ((c = get_char()) != '"') {
            // Check for escape sequence.
            if (c == '\\') {
                // Skip two symbols, backslash and following escaped symbol.
                eat_char();
                eat_char();
                length += 2;
                continue;
            }

            // Newlines are not allowed to intercept string.
            if (c == '\0' || is_newline(c)) {
                restore_checkpoint(checkpoint);

                // auto line = get_line();

                // printf("%s:%lu:%lu: error: unclosed double quote\n", filename, checkpoint.line_counter, checkpoint.char_counter);
                // printf("   %lu |%.*s\n", checkpoint.line_counter, (int)line.length, line.start);
                // printf("   %.*s |%.*s%.*s\n",
                //     (int)(digits_in_number(checkpoint.line_counter)), spaces,
                //     (int)(checkpoint.current - line.start), spaces,
                //     (int)(line.length - (checkpoint.current - line.start)), carets);
                return false;
            }

            // If it is not an escape symbol, just skip it once.
            eat_char();
            length += 1;
        }

        eat_char(); // Skip double quote.
        length += 1;

        token t;
        t.in_text.begin = checkpoint.current_char;
        t.in_text.size = length;
        t.line_number = checkpoint.line_counter;
        t.char_number = checkpoint.char_counter;
        t.kind = TOKEN_STRING;
        t.value.integer = 0;

        token_stream.push_back(t);
        return true;
    }

    bool eat_keyword_or_identifier () {
        char c = get_char();

        if (!is_valid_identifier_head(c)) {
            return false;
        }

        // This have to eat at least one symbol.
        auto result = eat_while(is_valid_identifier_body);

        auto token_kind = keywords.find(std::string(result.begin, result.size));
        if (token_kind == keywords.end()) {
            // This is an identifier

            token t;
            t.in_text.begin = result.begin;
            t.in_text.size = result.size;
            t.line_number = state.line_counter;
            t.char_number = state.char_counter;
            t.kind = TOKEN_IDENTIFIER;
            t.value.integer = 0;

            token_stream.push_back(t);
            return true;
        }

        token t;
        t.in_text = result;
        t.line_number = state.line_counter;
        t.char_number = state.char_counter;
        t.kind = token_kind->second;
        t.value.integer = 0;

        token_stream.push_back(t);
        return true;
    }

    bool eat_number () {
        const char* start = state.current_char;
        uint64_t len = 0;

        char c = get_char();
        if (!is_digit(c) && c != '.' && c != '-' && c != '+') {
            return false;
        }

        int64_t sign = 1;
        int64_t integral = 0;
        double fractional = 0.;
        double multiplier = 1.;

        if (c == '-' || c == '+') {
            sign = (c == '-' ? -1 : 1);
            eat_char();
            len += 1;
        }

        while (is_digit(c = get_char())) {
            eat_char();
            len += 1;

            integral *= 10;
            integral += (c - '0');
        }

        if (c == '.') {
            eat_char();
            len += 1;

            while(is_digit(c = get_char())) {
                eat_char();
                len += 1;

                multiplier *= 0.1;
                fractional += (c - '0') * multiplier;
            }

            token t;
            t.in_text.begin = start;
            t.in_text.size = len;
            t.line_number = state.line_counter;
            t.char_number = state.char_counter;
            t.kind = TOKEN_FLOATING;
            t.value.floating = sign*(integral + fractional);

            token_stream.push_back(t);
            return true;
        }

        token t;
        t.in_text.begin = start;
        t.in_text.size = len;
        t.line_number = state.line_counter;
        t.char_number = state.char_counter;
        t.kind = TOKEN_INTEGER;
        t.value.integer = sign*integral;

        token_stream.push_back(t);
        return true;
    }
};


std::string read_whole_file(const char *filename) {
    std::ifstream input(filename, std::ios::in | std::ios::binary);
    std::ostringstream content;

    if (input.good()) {
        content << input.rdbuf();
    }

    return content.str();
}


struct parser_impl {
    std::deque<token> token_stream;
    std::deque<token>::iterator it;

    son parse_array(bool top_level = false) {
        auto checkpoint = it;

        son result(son::type_t::array);
        bool have_open_bracket = false;

        {
            token t = *it;

            if (t.kind != TOKEN_BRACKET_OPEN and !top_level) {
                // report error
                // "%s:%lu:%lu: error: ’[’ is expected, found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            have_open_bracket = t.kind == TOKEN_BRACKET_OPEN;
            if (have_open_bracket) {
                // t_bracket_open = t;
                it++;
            }
        }

middle:
        do {
            {
                token t = *it;

                switch (t.kind) {
                    case TOKEN_KW_NULL: {
                        result.push(son());
                        it++;
                        break;
                    }
                    case TOKEN_KW_TRUE: {
                        result.push(true);
                        it++;
                        break;
                    }
                    case TOKEN_KW_FALSE: {
                        result.push(false);
                        it++;
                        break;
                    }
                    case TOKEN_INTEGER: {
                        result.push(t.value.integer);
                        it++;
                        break;
                    }
                    case TOKEN_FLOATING: {
                        result.push(t.value.floating);
                        it++;
                        break;
                    }
                    case TOKEN_STRING: {
                        result.push(std::string(t.in_text.begin + 1, t.in_text.size - 2));
                        it++;
                        break;
                    }
                    case TOKEN_BRACE_OPEN: {
                        // This is an object
                        son object = parse_object(false);

                        if (object.is_null()) {
                            it = checkpoint;
                            return son();
                        }

                        result.push(object);
                        break;
                    }
                    case TOKEN_BRACKET_OPEN: {
                        // This is a nested array
                        son array = parse_array(false);

                        if (array.is_null()) {
                            it = checkpoint;
                            return son();
                        }

                        result.push(array);
                        break;
                    }
                    case TOKEN_BRACKET_CLOSE: // If empty list
                        break;
                    default:
                        // error_t error;
                        // eprintf(error, "%s:%lu:%lu: error: ’]’ is expected, found %s ’%.*s’\n",

                        it = checkpoint;
                        return son();
                }
            }

            {
                token t = *it;

                if (t.kind == TOKEN_COMMA) {
                    it++;
                }

                if (t.kind == TOKEN_BRACKET_CLOSE or t.kind == TOKEN_EOF) {
                    break;
                }
            }
        } while (true);

        {
            token t = *it;

            if (have_open_bracket and t.kind == TOKEN_BRACKET_CLOSE) {
                // Consume ']'
                it++;
            }
            else if (!have_open_bracket and t.kind == TOKEN_BRACKET_CLOSE) {
                // error_t error;
                // eprintf(error, "%s:%lu:%lu: error: expected EOF (end of naked top level list), found %s ’%.*s’\n",
                it = checkpoint;
                return son();
            }
            else if (have_open_bracket and t.kind != TOKEN_BRACKET_CLOSE) {
                // error_t error;
                // eprintf(error, "%s:%lu:%lu: error: ’]’ is expected, found %s ’%.*s’\n",
                it = checkpoint;
                return son();
            }
        }

        {
            token t = *it;
            if (t.kind != TOKEN_EOF and top_level) {
                son top_level_list;
                top_level_list.push(result);
                result = top_level_list;
                have_open_bracket = false;
                goto middle; // @FIX BAD BAD BAD!!!
            }
        }

        return result;
    }

    bool parse_key_value_pair(std::string& key, son& value, bool top_level) {
        auto checkpoint = it;

        {
            token t = *it;

            if (top_level and t.kind == TOKEN_EOF) { return false; }
            if (not top_level and t.kind == TOKEN_BRACE_CLOSE) { return false; }
            if (t.kind != TOKEN_IDENTIFIER) {
                // report error
                // "%s:%lu:%lu: error: expected identifier, but found %s ’%.*s’\n"

                it = checkpoint;
                return false;
            }

            key = std::string(t.in_text.begin, t.in_text.size);
            it++;
        }

        {
            token t = *it;

            if (t.kind != TOKEN_EQUAL_SIGN) {
                // report error
                // "%s:%lu:%lu: error: expected ’=’, but found %s ’%.*s’\n"

                it = checkpoint;
                return false;
            }

            it++;
        }

        {
            token t = *it;

            switch (t.kind) {
            case TOKEN_KW_NULL: {
                value = son();
                it++;
                break;
            }
            case TOKEN_KW_TRUE: {
                value = son(true);
                it++;
                break;
            }
            case TOKEN_KW_FALSE: {
                value = son(false);
                it++;
                break;
            }
            case TOKEN_INTEGER: {
                value = son(t.value.integer);
                it++;
                break;
            }
            case TOKEN_FLOATING: {
                value = son(t.value.floating);
                it++;
                break;
            }
            case TOKEN_STRING: {
                value = son(std::string(t.in_text.begin + 1, t.in_text.size - 2));
                it++;
                break;
            }
            case TOKEN_BRACE_OPEN: {
                son object = parse_object(false);
                if (object.is_null()) {
                    // report error
                    // "%s:%lu:%lu: error: value is expected, found %s ’%.*s’\n"

                    it = checkpoint;
                    return false;
                }

                value = std::move(object);
                break;
            }
            case TOKEN_BRACKET_OPEN: {
                son array = parse_array(false);
                if (array.is_null()) {
                    // report error
                    // "%s:%lu:%lu: error: value is expected, found %s ’%.*s’\n"

                    it = checkpoint;
                    return false;
                }

                value = array;
                break;
            }
            default: {
                // report error
                // "%s:%lu:%lu: error: value is expected, found ’%.*s’\n"

                it = checkpoint;
                return false;
            }
            }
        }

        {
            token t = *it;

            if (t.kind == TOKEN_SEMICOLON) {
                it++; // Skip semicolon.
            } else {
                // Semicolon is optional.
            }
        }

        // We reached the end, it's all good.
        return true;
    }

    son parse_object(bool top_level = false) {
        auto checkpoint = it;
        bool have_open_brace = false;

        son result;

        {
            token t = *it;
            if (!(t.kind == TOKEN_BRACE_OPEN or
                 (t.kind == TOKEN_IDENTIFIER and top_level))) {
                // "%s:%lu:%lu: error: '{' expected, found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            have_open_brace = t.kind == TOKEN_BRACE_OPEN;

            if (t.kind == TOKEN_BRACE_OPEN) {
                it++;
            }
        }

        {
            do {
                token t = *it;

                if (t.kind != TOKEN_IDENTIFIER) break;

                std::string key;
                son value;
                if (!parse_key_value_pair(key, value, top_level)) break;

                result.push(key, value);
            } while (true);
        }

        {
            token t = *it;

            if (top_level and have_open_brace and t.kind != TOKEN_BRACE_CLOSE) {
                // report error
                // "%s:%lu:%lu: error: '}' expected, found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            if (top_level and !have_open_brace and t.kind != TOKEN_EOF) {
                // report error
                // "%s:%lu:%lu: error: expected EOF (end of naked top level object), but found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            if (top_level and have_open_brace and t.kind != TOKEN_BRACE_CLOSE) {
                // report error
                // "%s:%lu:%lu: error: '}' expected, found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            if (top_level and !have_open_brace and t.kind != TOKEN_EOF) {
                // report error
                // "%s:%lu:%lu: error: expected EOF (end of naked top level object), but found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            if (!top_level and !have_open_brace) {
                assert(false); // I don't know is this even possible
            }

            if (have_open_brace and t.kind != TOKEN_BRACE_CLOSE) {
                // report error
                // "%s:%lu:%lu: error: '}' expected, found %s ’%.*s’\n"

                it = checkpoint;
                return son();
            }

            if (t.kind == TOKEN_BRACE_CLOSE) {
                it++; // Consume '}'
            }
        }

        return result;
    }
};


son parse_impl(lexer& lex) {
    auto begin = lex.token_stream.begin();
    auto end = lex.token_stream.end();
    
    parser_impl parser;
    parser.token_stream = std::move(lex.token_stream);
    parser.it = parser.token_stream.begin();

    son obj = parser.parse_object(true);

    if (!obj.is_null()) {
        return obj;
    }

    begin = lex.token_stream.begin();
    end = lex.token_stream.end();

    son arr = parser.parse_array(true);

    return arr;
}


son parser::parse() {
    lexer lex;
    lex.filename = filename.c_str();

    std::string text = read_whole_file(filename.c_str());
    lex.text.begin = text.data();
    lex.text.size = text.size();

    lex.state.current_char = lex.text.begin;
    lex.state.current_line = lex.text.begin;

    lex.tokenize();

    return parse_impl(lex);
}

};

