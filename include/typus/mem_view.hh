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
#ifndef TYPUS_MEM_VIEW_HH
#define TYPUS_MEM_VIEW_HH

#include <cassert>
#include <cmath>
#include <ostream>


namespace typus {

/**
 * \brief A view for a continuous block of memory
 *
 * This class is a replacement for accessing memory through raw-pointers. 
 * The memory area is not owned by the mem_view. Thus it just be guaranteed by 
 * the caller that the memory region pointed to by mem_view is valid as long as 
 * it is accessed.
 */
template <typename T>
class mem_view {
public:
    mem_view(T* begin, T* end): begin_(begin), end_(end) { }
    mem_view() = default;
    mem_view(const mem_view &rhs) = default;
    mem_view &operator=(const mem_view &rhs) = default;

public:
    T* begin() { return begin_; }
    const T* begin() const { return begin_; }

    T* end() { return end_; }
    const T* end() const { return end_; }

    std::size_t size() const { return end_ - begin_; }

    bool empty() const { return end_ == begin_; }

    const T& operator[](std::size_t index) const {
        assert(begin_ + index < end_ && begin_ + index >= begin_);
        return begin_[index];
    }

    T& operator[](std::size_t index) {
        assert(begin_ + index < end_ && begin_ + index >= begin_);
        return begin_[index];
    }
public:
    bool operator==(const mem_view &rhs) const {
        if (this->size() != rhs.size()) {
            return false;
        }
        T* a = begin_;
        T* b = rhs.begin_;
        for (; a != end_; ++a, ++b) {
            if (*a != *b) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const mem_view &rhs) const {
        return !this->operator==(rhs);
    }
private:
    T* begin_ = nullptr;
    T* end_ = nullptr;
};

} // namespace

#endif // TYPUS_MEM_VIEW_HH
