#include <value.hpp>


namespace jslavic {


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
        case type_t::null:
        case type_t::boolean: 
        case type_t::integer: 
        case type_t::floating:
            break;
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


son::son(const char* s) noexcept {
    m_type = type_t::string;
    m_value.storage = new string_t(s);
}


son::son(std::initializer_list<son>) noexcept {
    // @todo
}


son::son(const son& other) noexcept {
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


void son::push(const char* key, const son& value) {
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


} // jslavic
