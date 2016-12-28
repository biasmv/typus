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
#ifndef TYPUS_FORMAT_HH_
#define TYPUS_FORMAT_HH_

#include <string>
#include <sstream>
#include <tuple>
#include "meta/char_sequence.hh"

namespace typus {

// contains the implementation for the format string parsing
namespace format_impl {

/**
 * A simple parameter pack of printable types.
 */
template <typename... T>
struct format_str { };

template <char... Cs>
using char_seq = ::typus::meta::char_sequence<Cs...>;

// FIXME: provide better impl that understands format options.
struct placeholder {};

// appends character to existing previous char_sequence by inserting the type at the 
// front (due to type deduction rules, we can't match types at the end of a variadic 
// argument pack). In case previous  type in the format_str was not a char sequence, 
// a new char_sequence is started.
template <char C, char... Cs, typename... Ts>
auto append_format(format_str<char_seq<Cs...>, Ts...>, char_seq<C>)
    -> format_str<char_seq<Cs..., C>, Ts...>;

// appends an placeholder
template <typename... Ts>
auto append_format(format_str<Ts...>, placeholder) -> format_str<placeholder, Ts...>;

// appends a char_sequence
template <char C, typename... Ts>
auto append_format(format_str<Ts...>, char_seq<C>) -> format_str<char_seq<C>, Ts...>;

// reverses the order of the argument packs of a format string
template <typename... Ts>
auto reverse(format_str<Ts...>) 
    -> decltype(reverse_impl(format_str<>{}, format_str<Ts...>{}));

template <typename T, typename... Ts, typename... Os>
auto reverse_impl(format_str<Os...>, format_str<T, Ts...>) 
    -> decltype(reverse_impl(format_str<T, Os...>{}, format_str<Ts...>{}));

template <typename... Os>
auto reverse_impl(format_str<Os...>, format_str<>) -> format_str<Os...>;

template <size_t W> struct width_f { };

template <char C> struct fill_f {};

template <typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<'{', '{', Cs...>) 
    -> decltype(parse_impl(append_format(format_str<Ts...>{}, char_seq<'{'>{}), 
                           char_seq<Cs...>{}));

template <char... Cs>
char_seq<Cs...> as_char_seq();

template <>
char_seq<> as_char_seq();

// parses a format-specifier or escaped {. Continues until either a second { is found 
// or the format specifier is terminated with }. For now, we don't do anything with 
// the format specifier.
template <typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<'{', Cs...>) 
    -> decltype(parse_impl(append_format(format_str<Ts...>{}, placeholder{}), 
                           as_char_seq<Cs...>()));

template <typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<'}', '}', Cs...>) 
    -> decltype(parse_impl(append_format(format_str<Ts...>{}, char_seq<'}'>{}), 
                           as_char_seq<Cs...>()));

template <typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<'}', Cs...>) 
    -> decltype(parse_impl(format_str<Ts...>{}, 
                           as_char_seq<Cs...>()));

// char_sequence, whereas placeholders are stored as separate types.
template <char C, typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<C, Cs...>) 
    -> decltype(parse_impl(append_format(format_str<Ts...>{}, char_seq<C>{}), 
                           as_char_seq<Cs...>()));


template <typename... Ts, char... Cs>
auto parse_impl(format_str<Ts...>, char_seq<Cs...>) -> format_str<Ts...>;

// the return value of parse_impl contains the arguments in reverse order. Before we
// can use them we have to flip em.
template <char... Cs>
auto parse(char_seq<Cs...>) -> 
    decltype(reverse(parse_impl(format_str<>{}, char_seq<Cs...>{})));

template <typename T, typename... Ts>
std::ostream & operator<<(std::ostream &os, const format_str<T, Ts...> &placeholder) {
    return os << T{} << format_str<Ts...>{};
}

// matches empty template arg pack -> terminate recursion
template <typename ... Ts>
std::ostream & operator<<(std::ostream &os, const format_str<Ts...> &placeholder) { 
    return os; 
}

template <size_t N, typename... A, char... Cs, typename... T>
inline void print_impl(std::ostream &os, 
                const format_str<char_seq<Cs...>, A...>, 
                const std::tuple<T...> &args) {
    os << char_seq<Cs...>{};
    print_impl<N>(os, format_str<A...>{}, args);
}

template <size_t N, typename... A, typename... T>
inline void print_impl(std::ostream &os, 
                const format_str<placeholder, A...>, 
                const std::tuple<T...> &args) {
    static_assert(N < sizeof...(T), "not enough arguments provided for format string");
    os << std::get<N>(args);
    print_impl<N+1>(os, format_str<A...>{}, args);
}

template <size_t N, typename... A, typename... T>
inline void print_impl(std::ostream &os, const format_str<A...>, 
                       const std::tuple<T...> &args) {
    // ensure that all arguments have been used.
    static_assert(N == sizeof...(T), "not all arguments were used in format string");
}

} // ns format_impl

template <typename F>
struct format {
    format() = default;

    template <typename... A>
    std::ostream &into_stream(std::ostream &stream, A&& ...args) {
        format_impl::print_impl<0>(stream, F{}, std::forward_as_tuple<>(args...));
        return stream;
    }

    template <typename... A>
    std::string to_str(A && ...args) {
        std::stringstream ss;
        this->into_stream(ss, args...);
        return ss.str();
    }
};

/**
 * \brief Creates a format string, capable of printing a series of arguments into 
 *     either a stream or a string.
 *
 * The format is parsed at compile time. It is possible to reuse the same format 
 * string multiple times, e.g. when printing a series of values into a file.
 *
 * FIXME: is there away to avoid using a macro here? 
 */
#define $(x) typus::format<decltype(typus::format_impl::parse(TYPUS_CHAR_SEQ(x){}))>{}

} // ns typus

#endif
