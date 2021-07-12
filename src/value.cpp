#include <value.hpp>
#include <algorithm>
#include <unordered_map>
#include <inttypes.h>


namespace jslavic {


struct custom_type_factory {
    std::unordered_map<std::string, void*> constructors;

    template <typename T, T(*F)()>
    void register_new_type(const std::string& name) {
        constructors.emplace(name, F);
    }

    void* get_constructor_of(const std::string& name) {
        auto it = constructors.find(name);
        if (it == constructors.end()) {
            return nullptr;
        }

        return it->second;
    }
};


son::~son() {
    switch (m_type) {
        case type_t::null:
        case type_t::boolean:
        case type_t::integer:
        case type_t::floating:
            break;
        case type_t::string: delete (string_t*)m_value.storage; break;
        case type_t::object: delete (object_t*)m_value.storage; break;
        case type_t::array:  delete (array_t*)m_value.storage;  break;
        // case type_t::custom: // @todo
    }
}


son::son()
    : m_type(type_t::null)
{
    m_value.storage = nullptr;
}


son::son(std::nullptr_t) noexcept
    : son()
{}


son::son(type_t t) noexcept
    : son()
{
    m_type = t;
    switch (t) {
    case type_t::null: m_value.storage = nullptr; break;
    case type_t::boolean: m_value.boolean = false; break;
    case type_t::integer: m_value.integer = 0; break;
    case type_t::floating: m_value.floating = 0.0; break;
    case type_t::string: m_value.storage = new string_t(); break;
    case type_t::object: m_value.storage = new object_t(); break;
    case type_t::array:  m_value.storage = new array_t();  break;
    // case type_t::custom: // @todo
    }
}


son::son(boolean_t v) noexcept {
    m_type = type_t::boolean;
    m_value.boolean = v;
}


son::son(integer_t v) noexcept {
    m_type = type_t::integer;
    m_value.integer = v;
}


son::son(floating_t v) noexcept {
    m_type = type_t::floating;
    m_value.floating = v;
}


son::son(const char* s) noexcept
    : son(std::string(s))
{}


son::son(std::string s) noexcept {
    m_type = type_t::string;
    m_value.storage = new string_t(std::move(s));
}


son::son(std::initializer_list<son> init_list) noexcept
    : son()
{
    bool is_an_object = std::all_of(init_list.begin(), init_list.end(),
        [](const son& v) -> bool {
            return v.is_array() && v.size() == 2 && v[0].is_string();
        });

    if (is_an_object) {
        for (auto& v : init_list) {
            push(v[0].get_string(), v[1]);
        }
    } else {
        for (auto& v : init_list) {
            push(v);
        }
    }
}


son::son(const son& other) noexcept
    : son()
{
    m_type = other.m_type;

    switch (m_type) {
        case type_t::null:
            break;
        case type_t::boolean:
            m_value.boolean = other.m_value.boolean;
            break;
        case type_t::integer:
            m_value.integer = other.m_value.integer;
            break;
        case type_t::floating:
            m_value.floating = other.m_value.floating;
            break;
        case type_t::string: {
            string_t* copy = new string_t(*(string_t*)other.m_value.storage);
            m_value.storage = copy;
            break;
        }
        case type_t::object: {
            object_t* copy = new object_t(*(object_t*)other.m_value.storage);
            m_value.storage = copy;
            break;
        }
        case type_t::array: {
            array_t* copy = new array_t(*(array_t*)other.m_value.storage);
            m_value.storage = copy;
            break;
        }
    }
}


son::son(son&& other) noexcept {
    this->swap(other);
}


son& son::operator=(const son& other) noexcept {
    son(other).swap(*this);
    return *this;
}


son& son::operator=(son&& other) noexcept {
    other.swap(*this);
    return *this;
}


void son::swap(son& other) noexcept {
    std::swap(m_type, other.m_type);
    std::swap(m_value, other.m_value);
}


bool son::operator==(const son& other) const {
    if (type() != other.type()) return false;

    switch (type()) {
    case type_t::null: return true;
    case type_t::boolean: return get_boolean() == other.get_boolean();
    case type_t::integer: return get_integer() == other.get_integer();
    case type_t::floating: return get_floating() == other.get_floating();
    case type_t::string: {
        string_t* p_storage = (string_t*)m_value.storage;
        string_t* p_other_storage = (string_t*)other.m_value.storage;

        return (*p_storage) == (*p_other_storage);
    }
    case type_t::object: {
        object_t* p_storage = (object_t*)m_value.storage;
        object_t* p_other_storage = (object_t*)other.m_value.storage;

        return (*p_storage) == (*p_other_storage);
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)m_value.storage;
        array_t* p_other_storage = (array_t*)other.m_value.storage;

        return (*p_storage) == (*p_other_storage);
    }
    }

    // Why gcc says that control reaches end of non-void function,
    // if there's return in every possible case above?
    return false;
}


son& son::operator[](const char* key) {
    assert(is_null() || is_object());

    if (is_null()) {
        push(key, son());
    }

    object_t* p_storage = (object_t*)m_value.storage;

    for (auto& pair : (*p_storage)) {
        if (pair.first == std::string(key)) {
            return pair.second;
        }
    }

    push(key, son());

    return (*p_storage)[p_storage->size() - 1].second;
}


son& son::operator[](int32_t idx) {
    assert(is_array());

    array_t* p_storage = (array_t*)m_value.storage;
    return (*p_storage)[idx];
}


const son& son::operator[](const char* key) const {
    assert(is_null() || is_object());

    son* self = const_cast<son*>(this);

    if (is_null()) {
        self->push(key, son());
    }

    object_t* p_storage = (object_t*)m_value.storage;

    for (auto& pair : (*p_storage)) {
        if (pair.first == std::string(key)) {
            return pair.second;
        }
    }

    self->push(key, son());

    return (*p_storage)[p_storage->size() - 1].second;
}


const son& son::operator[](int32_t idx) const {
    assert(is_array());

    array_t* p_storage = (array_t*)m_value.storage;
    return (*p_storage)[idx];
}


son son::get(const char* key, const son& default_value) {
    assert(is_object());
    object_t* p_storage = (object_t*)m_value.storage;

    for (auto& pair : (*p_storage)) {
        if (pair.first == std::string(key)) {
            if (pair.second.is_null()) return default_value;
            return pair.second;
        }
    }

    return default_value;
}


son son::get(int32_t idx, const son& default_value) {
    assert(is_array());
    array_t* p_storage = (array_t*)m_value.storage;

    if (static_cast<size_t>(idx) < p_storage->size()) {
        return (*p_storage)[idx];
    }

    return default_value;
}


void son::push(const std::string& key, const son& value) {
    assert(is_null() || is_object());

    if (is_null()) {
        son obj(type_t::object);
        this->swap(obj);
    }

    object_t* p_storage = (object_t*)m_value.storage;
    p_storage->emplace_back(key, value);
}


void son::push(const son& value) {
    assert(is_null() || is_array());

    if (is_null()) {
        son arr(type_t::array);
        this->swap(arr);
    }

    array_t* p_storage = (array_t*)m_value.storage;
    p_storage->push_back(value);
}


bool son::empty() const {
    switch (type()) {
    case type_t::null: return true;
    case type_t::boolean:
    case type_t::integer:
    case type_t::floating:
    case type_t::string:
        return false;
    case type_t::object: {
        object_t* p_storage = (object_t*)m_value.storage;
        return p_storage->empty();
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)m_value.storage;
        return p_storage->empty();
    }
    }

    // Why gcc says that control reaches end of non-void function,
    // if there's return in every possible case above?
    // What should I return then? I don't know, it doesn't make sense.
    // Try to return 0, to match fallthrough of size() function, so
    // it'll be empty and of size 0.
    return true;
}


size_t son::size() const {
    switch (type()) {
    case type_t::null: return 0;
    case type_t::boolean:
    case type_t::integer:
    case type_t::floating:
    case type_t::string:
        return 1;
    case type_t::object: {
        object_t* p_storage = (object_t*)m_value.storage;
        return p_storage->size();
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)m_value.storage;
        return p_storage->size();
    }
    }

    // Why gcc says that control reaches end of non-void function,
    // if there's return in every possible case above?
    return 0;
}


size_t son::deep_size() const {
    switch (type()) {
    case type_t::null:
    case type_t::boolean:
    case type_t::integer:
    case type_t::floating:
    case type_t::string:
        return 1;
    case type_t::object: {
        size_t n = 0;
        object_t* p_storage = (object_t*)m_value.storage;
        for (auto& [k, v] : (*p_storage)) {
            n += 1; // for key
            n += v.deep_size();
        }
        return n;
    }
    case type_t::array: {
        size_t n = 0;
        array_t* p_storage = (array_t*)m_value.storage;
        for (auto& v : (*p_storage)) {
            n += v.deep_size();
        }
        return n;
    }
    }

    // Why gcc says that control reaches end of non-void function,
    // if there's return in every possible case above?
    return 0;
}


void son::clear() {
    switch (type()) {
    case type_t::null: return;
    case type_t::boolean: m_value.boolean = false; return;
    case type_t::integer: m_value.integer = 0; return;
    case type_t::floating: m_value.floating = 0.0; return;
    case type_t::string: {
        string_t* p_storage = (string_t*)m_value.storage;
        return p_storage->clear();
    }
    case type_t::object: {
        object_t* p_storage = (object_t*)m_value.storage;
        return p_storage->clear();
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)m_value.storage;
        return p_storage->clear();
    }
    }
}


son& son::iterator::operator * () {
    switch (p->m_type) {
        case type_t::object: {
            object_t* p_storage = (object_t*)p->m_value.storage;
            return (*p_storage)[idx].second;
        }
        case type_t::array: {
            array_t* p_storage = (array_t*)p->m_value.storage;
            return (*p_storage)[idx];
        }
        default: return *p;
    }
}


void son::iterator::set_to_end() {
    switch (p->m_type) {
        case type_t::null:
        case type_t::boolean:
        case type_t::integer:
        case type_t::floating:
        case type_t::string:
            idx = 1;
            break;
        case type_t::object: {
            object_t* p_storage = (object_t*) p->m_value.storage;
            idx = p_storage->size();
            break;
        }
        case type_t::array: {
            array_t* p_storage = (array_t*) p->m_value.storage;
            idx = p_storage->size();
            break;
        }
    }
}


const son& son::const_iterator::operator * () const {
    switch (p->m_type) {
    case type_t::object: {
        object_t* p_storage = (object_t*)p->m_value.storage;
        return (*p_storage)[idx].second;
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)p->m_value.storage;
        return (*p_storage)[idx];
    }
    default: return *p;
    }
}


void son::const_iterator::set_to_end() {
    switch (p->m_type) {
    case type_t::null:
    case type_t::boolean:
    case type_t::integer:
    case type_t::floating:
    case type_t::string:
        idx = 1;
        break;
    case type_t::object: {
        object_t* p_storage = (object_t*)p->m_value.storage;
        idx = p_storage->size();
        break;
    }
    case type_t::array: {
        array_t* p_storage = (array_t*)p->m_value.storage;
        idx = p_storage->size();
        break;
    }
    }
}


static const char* spaces = "                                                  ";
int32_t pretty_print_impl(son& value, const print_options& options, int32_t depth) {
    // @Fix: The reason that value is non-constant type is that I didn't make pairs work with constant iterators.
    //       To fix this, make const_object_iterator, const_iterator_proxy, and make pairs which will work with constant types.
    switch (value.type()) {
    case son::type_t::null: fprintf(options.output, "null"); break;
    case son::type_t::boolean: fprintf(options.output, "%s", value.get_boolean() ? "true" : "false"); break;
    case son::type_t::integer: fprintf(options.output, "%" PRId64, value.get_integer()); break;
    case son::type_t::floating: fprintf(options.output, "%lf", value.get_floating()); break;
    case son::type_t::string: fprintf(options.output, "\"%s\"", value.get_string().c_str()); break;
    case son::type_t::object: {
        bool in_one_line = (options.multiline == print_options::multiline_t::smart && value.deep_size() <= 6)
            || options.multiline == print_options::multiline_t::disabled;

        fprintf(options.output, "{%s", in_one_line ? " " : "\n");
        depth += 1;

        for (auto p : value.pairs()) {
            auto& k = p.first;
            auto& v = p.second;

            if (!in_one_line) { fprintf(options.output, "%.*s", options.indent * depth, spaces); }
            fprintf(options.output, "%s = ", k.c_str());

            pretty_print_impl(v, options, depth);

            fprintf(options.output, "%s%s",
                options.print_semicolons ? ";" : "",
                in_one_line ? " " : "\n"
            );
        }

        depth -= 1;
        fprintf(options.output, "%.*s}", in_one_line ? 0 : options.indent * depth, spaces);
        break;
    }
    case son::type_t::array: {
        bool in_one_line = (options.multiline == print_options::multiline_t::smart && value.deep_size() <= 6)
            || options.multiline == print_options::multiline_t::disabled;

        fprintf(options.output, "[%s", in_one_line ? value.size() > 0 ? " " : "" : "\n");
        depth += 1;

        for (size_t i = 0; i < value.size(); i++) {
            auto& v = value[i];

            if (!in_one_line) { fprintf(options.output, "%.*s", options.indent * depth, spaces); }

            pretty_print_impl(v, options, depth);

            fprintf(options.output, "%s%s",
                options.print_commas && i + 1 < value.size() ? "," : "",
                in_one_line ? " " : "\n"
            );
        }

        depth -= 1;
        fprintf(options.output, "%.*s]", in_one_line ? 0 : options.indent * depth, spaces);
        break;
    }
    }

    return 0;
}


int32_t pretty_print(const son& value, const print_options& options /* = print_options()*/) {
    if (options.output == nullptr) { return -1; }
    return pretty_print_impl(const_cast<son&>(value), options, 0);
}


int32_t pretty_print(const char* fmt, const son& value, const print_options& options /* = print_options()*/) {
    const char* p = fmt;
    while (*p) {
        if (*p == '{' && *(p + 1) == '{') {
            fprintf(options.output, "%.*s{", (int32_t)((uint64_t)p - (uint64_t)fmt), fmt);
            p = p + 2;
            fmt = p;
            continue;
        }
        if (*p == '}' && *(p + 1) == '}') {
            fprintf(options.output, "%.*s}", (int32_t)((uint64_t)p - (uint64_t)fmt), fmt);
            p = p + 2;
            fmt = p;
            continue;
        }
        if (*p == '{' && *(p + 1) == '}') {
            fprintf(options.output, "%.*s", (int32_t)((uint64_t)p - (uint64_t)fmt), fmt);
            pretty_print_impl(const_cast<son&>(value), options, 0);
            p = p + 2;
            fmt = p;
            continue;
        }

        p++;
    }

    fprintf(options.output, "%.*s", (int32_t)((uint64_t)p - (uint64_t)fmt), fmt);

    return 0;
}


} // jslavic
