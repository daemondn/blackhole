#pragma once

#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/expression/helper.hpp"
#include "blackhole/filter.hpp"


namespace blackhole {

namespace expression {

template<typename T>
struct get_attr_action_t {
    typedef T result_type;

    const std::string name;

    const result_type& operator()(const log::attributes_t& attributes) const {
        return boost::get<T>(attributes.at(name).value);
        //
    }

    filter_t operator ==(const T& other) const {
        return aux::Eq<get_attr_action_t<T>>({ *this, other });
    }
};

template<typename T>
get_attr_action_t<T> get_attr(const std::string& name) {
    return get_attr_action_t<T>({ name });
}

template<typename T>
get_attr_action_t<typename T::type> get_attr(const T&) {
    return get_attr<typename T::type>(std::string(T::name()));
}

} // namespace expression

} // namespace blackhole