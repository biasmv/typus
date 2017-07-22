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

#ifndef TYPUS_MEMORY_OPS_HH
#define TYPUS_MEMORY_OPS_HH

#include <cassert>
#include <type_traits>
#include <initializer_list>


namespace typus {

namespace detail {

template <bool cond>
using true_or_false_t = 
    typename std::conditional<cond, std::true_type, std::false_type>::type;

template <typename T>
void destroy_range(T *begin, T * end, std::false_type) {
    for (; begin != end; ++begin) {
        begin->~T();
    }
}

template <typename T>
void destroy_range(T *begin, T * end, std::true_type) {
}

template <typename T>
T* uninitialized_move_and_destroy_range(T *begin, T* end, T* dst, std::true_type) {
    for (; begin != end; ++begin, ++dst) {
        new(dst) T(std::move(*begin));
        begin->~T();
    }
    return dst;
}

template <typename T>
T * uninitialized_move_and_destroy_range(T *begin, T* end, T* dst, std::false_type) {
    // fallback implementation using copy constructor
    for (; begin != end; ++begin, ++dst) {
        new(dst) T(*begin);
        begin->~T();
    }
    return dst;
}

}
template <typename T>
T * uninitialized_move_and_destroy_range(T *begin, T* end, T* dst) {
    using tag = detail::true_or_false_t<std::is_move_constructible<T>::value>;
    return detail::uninitialized_move_and_destroy_range(begin, end, dst, tag{});
}

// destroys the elements contained in the range begin, end. no-op for trivially 
// destructible types.
template <typename T>
inline void destroy_range(T *begin, T* end) {
    using tag = detail::true_or_false_t<std::is_trivially_destructible<T>::value>;
    detail::destroy_range(begin, end, tag{});
}

}

#endif // TYPUS_MEMORY_OPS_HH

