#ifndef SON_VALUE_HPP
#define SON_VALUE_HPP

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>
#include <tuple>


namespace jslavic {

class son {
public:
    enum class type_t : uint8_t {
        null,
        boolean,
        integer,
        floating,
        string,
        object,
        array,
        // custom, // @todo
    };

    using boolean_t = bool;
    using integer_t = uint64_t;
    using floating_t = double;
    using string_t = std::string;
    using object_t = std::vector<std::pair<std::string, son>>;
    using array_t = std::vector<son>;

private:
    union value_t {
        boolean_t boolean;
        integer_t integer;
        floating_t floating;
        void* storage;
    } m_value;

    type_t m_type = type_t::null;

public:
    ~son();

    son(); // null.
    son(std::nullptr_t) noexcept; // Also null.
    son(type_t t) noexcept; // Default value of that type.
    son(boolean_t v) noexcept;
    son(integer_t v) noexcept;
    son(int32_t v) noexcept : son(static_cast<integer_t>(v)) {}
    son(floating_t v) noexcept;
    son(const char* s) noexcept;
    son(std::initializer_list<son>) noexcept;

    son(const son& other) noexcept;
    son(son&& other) noexcept;

    son& operator=(const son& other) noexcept;
    son& operator=(son&& other) noexcept;

    void swap(son& other) noexcept;

    type_t type() const noexcept { return m_type; }

    bool is_null() const noexcept { return m_type == type_t::null; }
    bool is_boolean() const noexcept { return m_type == type_t::boolean; }
    bool is_integer() const noexcept { return m_type == type_t::integer; }
    bool is_floating() const noexcept { return m_type == type_t::floating; }
    bool is_string() const noexcept { return m_type == type_t::string; }
    bool is_object() const noexcept { return m_type == type_t::object; }
    bool is_array() const noexcept { return m_type == type_t::array; }
    // bool is_custom() const noexcept { return m_type == type_t::custom; }

    bool get_boolean() const { assert(is_boolean()); return m_value.boolean; }
    integer_t get_integer() const { assert(is_integer()); return m_value.integer; }
    floating_t get_floating() const { assert(is_floating()); return m_value.floating; }
    string_t get_string() const { assert(is_string()); return *(string_t*)m_value.storage; }

    bool operator==(const son& other) const;
    bool operator!=(const son& other) const { return !(*this == other); }

    son& operator[](const char* key);
    son& operator[](int32_t idx);

    son get(const char* key, const son& default_value);
    son get(int32_t idx, const son& default_value);

    void push(const char* key, const son& value);
    void push(const son& value);

    bool empty() const;
    size_t size() const;
    void clear();

    template <typename Iterator>
    struct iterator_proxy {
        son* p = nullptr;

        iterator_proxy(son* p) : p(p) {}

    public:
        Iterator begin() { return Iterator(p); }
        Iterator end() { auto it = Iterator(p); it.set_to_end(); return it; }
    };

    struct iterator {
        son* p = nullptr;
        size_t idx = 0;

        iterator(son* p) : p(p) {}
        void set_to_end();

    public:
        iterator& operator ++ () { ++idx; return *this; }
        iterator  operator ++ (int) { iterator old = *this; operator++(); return old; }

        iterator& operator -- () { --idx; return *this; }
        iterator  operator -- (int) { iterator old = *this; operator--(); return old; }

        bool operator == (const iterator& other) const { return p == other.p && idx == other.idx; }
        bool operator != (const iterator& other) const { return !(*this == other); }

        son& operator * ();
    };

    template <typename Iterator>
    struct object_iterator {
        Iterator it;

        object_iterator(son* p) : it(p) {}
        void set_to_end() { it.set_to_end(); }

    public:
        object_iterator& operator ++ () { ++it; return *this; }
        object_iterator  operator ++ (int) { object_iterator old = *this; operator++(); return old; }

        object_iterator& operator -- () { --it; return *this; }
        object_iterator  operator -- (int) { object_iterator old = *this; operator--(); return old; }

        bool operator == (const object_iterator& other) const { return it == other.it; }
        bool operator != (const object_iterator& other) const { return !(*this == other); }

        std::pair<std::string, son&> operator * () {
            assert(it.p->is_object());

            object_t* storage = (object_t*)it.p->m_value.storage;
            return { (*storage)[it.idx].first, (*storage)[it.idx].second };
        }

        std::string& key() const {
            assert(it.p->is_object());

            object_t* storage = (object_t*)it.p->m_value.storage;
            return storage[it.idx].first;
        }
        son& value() const { return *it; }
    };

    iterator begin() { return iterator(this); }
    iterator end() { auto it = iterator(this); it.set_to_end(); return it; }

    iterator_proxy<object_iterator<iterator>> pairs() { return iterator_proxy<object_iterator<iterator>>(this); }

    const char* type_name() const noexcept {
        switch (m_type) {
            case type_t::null: return "null";
            case type_t::boolean: return "boolean";
            case type_t::integer: return "integer";
            case type_t::floating: return "floating";
            case type_t::string: return "string";
            case type_t::object: return "object";
            case type_t::array: return "array";
            // case type_t::custom: return "custom";
        }
    }
};


struct print_options {
    enum class multiline_t {
        disabled,
        enabled,
        smart,
    };
    
    FILE* output = stdout;
    bool print_semicolons = false;
    bool print_commas = false;
    int32_t indent = 2;
    multiline_t multiline = multiline_t::smart;
};


int32_t pretty_print(const son& value, const print_options& options = print_options());


} // jslavic


#endif // SON_VALUE_HPP
