#ifndef SON_VALUE_HPP
#define SON_VALUE_HPP

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>
#include <tuple>


namespace jslavic {

class son {
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

    union value_t {
        boolean_t boolean;
        integer_t integer;
        floating_t floating;
        void* storage;
    } m_value;

    type_t m_type = type_t::null;

public:
    template <type_t TYPE>
    static son create() = delete;

    ~son();

    son(); // null.
    son(std::nullptr_t) noexcept; // Also null.
    son(type_t t) noexcept; // Default value of that type.
    son(boolean_t v) noexcept;
    son(integer_t v) noexcept;
    son(floating_t v) noexcept;
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

    bool operator==(const son& other);
    bool operator!=(const son& other) { return !(*this == other); }

    son& operator[](const char* key);
    son& operator[](int32_t idx);

    son get(const char* key, const son& default_value);
    son get(int32_t idx, const son& default_value);

    void push(const char* key, const son& value);
    void push(const son& value);

    size_t empty() const;
    size_t size() const;
    size_t clear() const;

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


template <> son son::create<son::type_t::null>();
template <> son son::create<son::type_t::boolean>();
template <> son son::create<son::type_t::integer>();
template <> son son::create<son::type_t::floating>();
template <> son son::create<son::type_t::string>();
template <> son son::create<son::type_t::object>();
template <> son son::create<son::type_t::array>();



} // jslavic


#endif // SON_VALUE_HPP
