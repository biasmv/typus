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


namespace typus {

/**
 * \brief A type-safe bitset for enums
 */
template <typename E>
class flags {
public:
    using flag_storage_type = typename std::underlying_type<E>::type;

    flags(): bits_(0) {}
    flags(const flags<E> &rhs) = default;
    explicit flags(E value): 
        bits_(reinterpret_cast<flag_storage_type&>(value)) {
    }


    flags<E> & operator |=(E value) {
        bits_ |= reinterpret_cast<flag_storage_type&>(value);
        return *this;
    }

    flags<E>  operator | (E value) {
        flags<E> rv(*this);
        rv |= value;
        return rv;
    }

    flag_storage_type bits() const {
        return bits_;
    }

    void clear() {
        bits_ = 0;
    }

    bool is_set(E value) const {
        flag_storage_type t = reinterpret_cast<flag_storage_type&>(value);
        return (bits_ & t) == t;
    }
private:
    flag_storage_type bits_;
};

}
#endif // TYPUS_FLAGS_HH

