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
#ifndef TYPUS_META_CHAR_SEQUENCE_HH_
#define TYPUS_META_CHAR_SEQUENCE_HH_

#include <string>
#include <ios>

namespace typus { namespace meta {

/**
 * A meta-type representing a variable-length, zero-terminated sequence of 
 * characters, offering index and size operations. Oh, and it can be printed 
 * into an ostream. 
 *
 * Example: The type \c char_sequence<'A', 'B', 'C'> holds the character 
 * sequence "ABC". Typically these char_sequence types are constructed through 
 * TYPUS_CHAR_SEQ which turns a string literal into a char_sequence type, which 
 * is much more convenient.
 */
template <char... C>
struct char_sequence final {
private:
    constexpr static char const values_[sizeof...(C)+1] {
        C..., '\0' 
    };
    constexpr static size_t const size_ = sizeof...(C);
public:
    constexpr const char* c_str() const { return *(&char_sequence<C...>::values_); }
    constexpr char operator[](size_t i) const {
        return values_[i];
    }
    constexpr size_t size() const { return size_; }
};

template <char... C>
constexpr char const char_sequence<C...>::values_[sizeof...(C)+1];


/*
// one or more chars left to unpack, print then go to next
template <char C, char... Cs>
std::ostream &operator<<(std::ostream &os, const char_sequence<C, Cs...> &cs) {
    return os << C << char_sequence<Cs...>{};
}
*/
// no more chars to unpack, terminate recursion
template <char... Cs>
std::ostream &operator<<(std::ostream &os, const char_sequence<Cs...> &cs) {
    return os << char_sequence<Cs...>{}.c_str();
}


/**
 * Helper function to for getting character at certain position, returns '\0' 
 * if the index is out of bounds
 */
template <size_t I, size_t N>
constexpr char char_at(const char (&a)[N]) noexcept {
    // disable array bounds access warnings for clang. There tend to be a ton of array 
    // index out of bounds messages to be printed if wrong arguments are provided to 
    // the format string. They are bogus of course and don't help to diagnose the 
    // problem.
#   pragma clang diagnostic push 
#   pragma clang diagnostic ignored "-Warray-bounds"
    return I < N ? a[I] : '\0';
#   pragma clang diagnostic pop 
}

// The following section of functions only exist for type-deduction and don't have 
// an implementation.

// Discard everything after the terminating nul-byte.
template <char... C1, char... C2>
auto discard_after_nul_byte(char_sequence<C1...>, char_sequence<'\0'>, char_sequence<C2>...) 
    -> char_sequence<C1...>;

template <char... C1, char C2, char... C3>
auto discard_after_nul_byte(char_sequence<C1...>, char_sequence<C2>, char_sequence<C3>...) 
    -> decltype(discard_after_nul_byte(char_sequence<C1..., C2>{}, char_sequence<C3>{}...));

template <char... C1>
auto discard_after_nul_byte(char_sequence<C1...>) 
    -> char_sequence<C1...>;

// turn each character into a single-char char_sequence
template <char... C>
auto apply_chars() -> decltype(discard_after_nul_byte(char_sequence<C>{}...));

// drop first character from char_sequence for use in a decltype expression
template <char C, char... Cs>
char_sequence<Cs...> drop_first(char_sequence<C, Cs...>);

#define TYPUS_MK_INDEX_016(s, n) \
        typus::meta::char_at<0x##n##0>(s), typus::meta::char_at<0x##n##1>(s), \
        typus::meta::char_at<0x##n##2>(s), typus::meta::char_at<0x##n##3>(s), \
        typus::meta::char_at<0x##n##4>(s), typus::meta::char_at<0x##n##5>(s), \
        typus::meta::char_at<0x##n##6>(s), typus::meta::char_at<0x##n##7>(s), \
        typus::meta::char_at<0x##n##8>(s), typus::meta::char_at<0x##n##9>(s), \
        typus::meta::char_at<0x##n##a>(s), typus::meta::char_at<0x##n##b>(s), \
        typus::meta::char_at<0x##n##c>(s), typus::meta::char_at<0x##n##d>(s), \
        typus::meta::char_at<0x##n##e>(s), typus::meta::char_at<0x##n##f>(s)

#define TYPUS_MK_INDEX_256(s) \
        TYPUS_MK_INDEX_016(s, 0), TYPUS_MK_INDEX_016(s, 1), \
        TYPUS_MK_INDEX_016(s, 2), TYPUS_MK_INDEX_016(s, 3), \
        TYPUS_MK_INDEX_016(s, 4), TYPUS_MK_INDEX_016(s, 5), \
        TYPUS_MK_INDEX_016(s, 6), TYPUS_MK_INDEX_016(s, 7), \
        TYPUS_MK_INDEX_016(s, 8), TYPUS_MK_INDEX_016(s, 9), \
        TYPUS_MK_INDEX_016(s, a), TYPUS_MK_INDEX_016(s, b), \
        TYPUS_MK_INDEX_016(s, c), TYPUS_MK_INDEX_016(s, d), \
        TYPUS_MK_INDEX_016(s, e), TYPUS_MK_INDEX_016(s, f)


/**
 * \brief Turns a string literal into a variable template type parametrized with 
 *    the  contents of the string literal.
 *
 * For example, the string literal \c "abc" is transformed into the type 
 * \c char_sequence<'a', 'b', 'c'>. This type can then be operated on at compile 
 * time.
 *
 * Implementation details:
 *
 * The process of transforming the string literal into a type works as follows:
 * 
 * - In the first step, individual characters are extracted from the string and 
 *   used as template arguments for an expression template. This happens through 
 *   macro substitution and the  use of a constexpr function. For each  string 
 *   the same number of characters are extracted. For characters that are past 
 *   the end of the string literal, the last character is returned (nul-byte would 
 *   work as well). This design allows to use the same macro for all strings, 
 *   irrespective of their length.
 * - In a second step, the extracted characters are passed through a set of 
 *   templated functions to returned a cleaned-up \c char_sequence<Cs...> type. 
 *   The templated functions are just declared, but not defined and are solely 
 *   used for pattern  matching during type deduction. They are never used at 
 *   run-time. These functions append consecutive characters to the string until 
 *   a nul-byte is found.
 *
 * Limitations:
 * - String length is currently limited to 256 (plus one nul-byte). 
 */
#define TYPUS_CHAR_SEQ(x) \
    decltype(typus::meta::apply_chars<TYPUS_MK_INDEX_256(x)>())

}} // ns typus::meta

#endif
