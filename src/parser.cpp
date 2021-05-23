#include <parser.hpp>
#include <deque>
#include <unordered_map>
#include <fstream>
#include <sstream>


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


static const char* to_string(kind_t k) {
    switch (k) {
        case TOKEN_UNDEFINED:     return "? undefined";

        case TOKEN_EQUAL_SIGN:    return "=";
        case TOKEN_SEMICOLON:     return ";";
        case TOKEN_COMMA:         return ",";

        case TOKEN_BRACE_OPEN:    return "{";
        case TOKEN_BRACE_CLOSE:   return "}";

        case TOKEN_PAREN_OPEN:    return "(";
        case TOKEN_PAREN_CLOSE:   return ")";

        case TOKEN_BRACKET_OPEN:  return "[";
        case TOKEN_BRACKET_CLOSE: return "]";

        case TOKEN_KW_NULL:       return "null";
        case TOKEN_KW_TRUE:       return "true";
        case TOKEN_KW_FALSE:      return "false";

        case TOKEN_IDENTIFIER:    return "identifier";
        case TOKEN_INTEGER:       return "integer";
        case TOKEN_FLOATING:      return "floating";
        case TOKEN_STRING:        return "string";

        case TOKEN_DOUBLE_SLASH:  return "//";

        case TOKEN_EOF:           return "EOF";
    }

    return "ERROR";
}


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


void print_token(token t) {
    printf("%lu:%lu token { kind = %20s; value = ", t.line_number, t.char_number, to_string(t.kind));

    switch (t.kind) {
        case TOKEN_UNDEFINED: printf("ERROR! }\n"); break;
        case TOKEN_EOF:       printf("EOF; }\n"); break;
        case TOKEN_KW_NULL:   printf("null; }\n"); break;
        case TOKEN_KW_TRUE:   printf("true; }\n"); break;
        case TOKEN_KW_FALSE:  printf("false; }\n"); break;
        case TOKEN_INTEGER:   printf("%ld; }\n", t.value.integer); break;
        case TOKEN_FLOATING:  printf("%lf; }\n", t.value.floating); break;

        case TOKEN_IDENTIFIER:
        case TOKEN_STRING:
            printf("%.*s; }\n", int(t.in_text.size), t.in_text.begin);
            break;
        
        case TOKEN_EQUAL_SIGN:
        case TOKEN_SEMICOLON:
        case TOKEN_COMMA:
        case TOKEN_BRACE_OPEN:
        case TOKEN_BRACE_CLOSE:
        case TOKEN_PAREN_OPEN:
        case TOKEN_PAREN_CLOSE:
        case TOKEN_BRACKET_OPEN:
        case TOKEN_BRACKET_CLOSE:
            printf("'%c'; }\n", char(t.kind));
            break;

        case TOKEN_DOUBLE_SLASH:
            printf("//; }\n");
            break;
        default:
            printf("??? }\n");
    }
}



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
                // t.line = get_line();
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
            // t.line = get_line();
            t.kind = TOKEN_EOF;
            t.value.integer = 0;

            token_stream.push_back(t);
            // bucket_push_token(storage, t);
            return true;
        }

        return false;
    }

    // @make that escaped newlines do not show up in resulted string
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
        // t.line = get_line();
        t.kind = TOKEN_STRING;
        t.value.integer = 0;

        token_stream.push_back(t);
        // bucket_push_token(storage, t);
        // p->push_token(t);
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
            // t.line = get_line();
            t.kind = TOKEN_IDENTIFIER;
            t.value.integer = 0;

            token_stream.push_back(t);
            return true;
        }

        token t;
        t.in_text = result;
        t.line_number = state.line_counter;
        t.char_number = state.char_counter;
        // t.line = get_line();
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
            // t.line = get_line();
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
        // t.line = get_line();
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


son parser::parse() {
    lexer l;
    l.filename = filename.c_str();

    std::string text = read_whole_file(filename.c_str());
    l.text.begin = text.data();
    l.text.size = text.size();

    l.state.current_char = l.text.begin;
    l.state.current_line = l.text.begin;

    l.tokenize();

    for (auto& t : l.token_stream) {
        print_token(t);
    }

    return son();
}

};

