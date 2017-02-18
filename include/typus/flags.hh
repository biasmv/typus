// -----------------------------------------------------------------------------
// Copyright 2016 Marco Biasini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// -----------------------------------------------------------------------------

#ifndef TYPUS_FLAGS_HH
#define TYPUS_FLAGS_HH

#include <type_traits>
#include <initializer_list>


namespace typus {

template <typename E>
class flags;

namespace details {


template <typename E>
constexpr typename flags<E>::flag_storage_type to_bits(const flags<E> &value) {
    return value.bits();
}

template <typename E>
constexpr typename std::underlying_type<E>::type to_bits(E value) {
    return static_cast<typename std::underlying_type<E>::type>(value);
}

template <typename T>
constexpr T fold_bitwise_or(T current) {
    return current;
}

template <typename T, typename E, typename ...Es>
constexpr T fold_bitwise_or(T current, E x, Es ...xs) {
    return fold_bitwise_or(current | to_bits(x), xs...);
}


}

// helper macro for constructing boolean ops for flags type. Adds both <op>= 
// and <op> to the class. For asymmetric operands, the swapped version must be 
// added as well outside the class definition.
#define TYPUS_BOOLEAN_OP(OP, TYPE)            \
    flags<E> & operator OP##=(TYPE value) {   \
        bits_ OP##= details::to_bits(value);  \
        return *this;                         \
    }                                         \
    flags<E>  operator OP (TYPE value) const {\
        flags<E> rv(*this);                   \
        rv OP##= value;                       \
        return rv;                            \
    }                                         \



/**
 * \brief A type-safe replacement plain int bit fields to store combinations 
 *    of enums.
 *
 * This class is typically used to hold one or more enum values that are power 
 * of two.
 */
template <typename E>
class flags {
public:
    using flag_storage_type = typename std::underlying_type<E>::type;

    constexpr flags(): bits_(0) {}

    constexpr flags(const flags<E> &rhs) = default;

    /**
     * \brief Construct flags by bitwise ORing of all of the arguments.
     *
     * Arguments must either be a of type flags<E>, or type E, where E is 
     * the enum type of the flags object to be constructed.
     */
    template<typename X, typename ...Xs>
    explicit constexpr flags(X x, Xs... xs): 
        bits_(details::fold_bitwise_or(details::to_bits(x), xs...))
    {
    }

    /**
     */
    flags(const std::initializer_list<E> & values): bits_(0) {
        for (E value: values) {
            bits_ |= details::to_bits(value);
        }
    }

    TYPUS_BOOLEAN_OP(|, E)
    TYPUS_BOOLEAN_OP(|, const flags<E> &)
    TYPUS_BOOLEAN_OP(&, E)
    TYPUS_BOOLEAN_OP(&, const flags<E> &)


    constexpr flag_storage_type bits() const {
        return bits_;
    }

    void clear_all() {
        bits_ = 0;
    }

    constexpr bool is_set(E value) const {
        return (bits_ & details::to_bits(value)) == details::to_bits(value);
    }
private:
    flag_storage_type bits_;
};


template <typename E>
inline flags<E> operator | (E lhs, const flags<E> & rhs) {
    return rhs | lhs;
}

template <typename E>
inline flags<E> operator & (E lhs, const flags<E> &rhs) {
    return rhs | lhs;
}

#undef TYPUS_BOOLEAN_OP

}
#endif // TYPUS_FLAGS_HH

