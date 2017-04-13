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

#ifndef TYPUS_SMALL_VECTOR_HH
#define TYPUS_SMALL_VECTOR_HH

#include <cstdint>
#include <type_traits>
#include <initializer_list>
#include <memory>

#include "assert.hh"
#include "memory_ops.hh"

namespace typus {

namespace detail {

template <typename T, std::size_t N=1>
using aligned_storage = 
    typename std::aligned_storage<sizeof(T) * N, 
                                  alignof(T)>::type;

template <typename T, std::size_t N>
class small_storage;

template <typename T>
class small_storage<T, 0> {};

template <typename T>
class small_storage<T, 1> {};

template <typename T, std::size_t N>
class small_storage : 
    public aligned_storage<T, N> {
};

// calculates the next power of two greater than the argument. (32-bit)
uint32_t next_power_of_two(uint32_t n) {
    // use s a sequence of bit-shift an bitwise or to set all the bits 
    // to one that are to the right of already set bits. The resulting 
    // numbers will be of the form ...00111111..., so adding one will 
    // generate the next-bigger power-of-two. This also works with 0.
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n + 1u;
}

// calculates the next power of two greater than the argument. (64-bit)
uint32_t next_power_of_two(uint64_t n) {
    // see comment in 32-bit version for how this works.
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    n |= (n >> 32);
    return n + 1u;
}

template <size_t N>
std::size_t sel_next_power_of_two(std::size_t n);

template <>
std::size_t sel_next_power_of_two<4>(std::size_t n) {
    return static_cast<std::size_t>(next_power_of_two(static_cast<uint32_t>(n)));
}

template <>
std::size_t sel_next_power_of_two<8>(std::size_t n) {
    return static_cast<std::size_t>(next_power_of_two(static_cast<uint64_t>(n)));
}

std::size_t next_power_of_two(std::size_t n) {
    // required for avoiding ambiguous function resolution
    return sel_next_power_of_two<sizeof(std::size_t)>(n);
}

}

/**
 * \brief A small vector of element type T. 
 *
 * This class can not be instantiated on its own, but it can be used in 
 * interfaces to type-erase the small vector size.
 */
template <typename T>
class small_vector {
protected:
    T* begin_;
    T* end_;
    T* capacity_;
    detail::aligned_storage<T> head_;
protected:
    small_vector(std::size_t size): 
        begin_(reinterpret_cast<T*>(&head_)), 
        end_(reinterpret_cast<T*>(&head_)), 
        capacity_(begin_ + size) {
    }

    template <typename I>
    small_vector(I begin, I end, std::size_t size):
      small_vector(size) {
      this->append(begin, end);
    }
public:

    ~small_vector() {
        destroy_range(begin_, end_);
        if (!this->is_small()) {
            free(begin_);
        }
    }

    inline bool is_small() const {
        return begin_ == reinterpret_cast<const T*>(&head_);
    }

    inline T* begin() { return begin_; }
    inline const T* begin() const { return begin_; }

    inline T* end() { return end_; }

    inline const T* end() const { return end_; }

    /**
     * \brief The current capacity of the vector
     */
    inline std::size_t capacity() const {
        return capacity_ - begin_;
    }

    /**
     * \brief The number of elements contained in the vector
     */
    inline std::size_t size() const {
        return end_ - begin_;
    }

    // returns true when size() == 0u
    inline bool empty() const {
        return begin_ == end_;
    }

    inline const T& operator[](std::size_t i) const {
        TYPUS_REQUIRES(i < this->size());
        return begin_[i];
    }

    inline T& operator[](std::size_t i) {
        TYPUS_REQUIRES(i < this->size());
        return begin_[i];
    }

    /**
     * \brief Access the first element of the vector.
     *
     * \pre The vector is not empty.
     */
    inline T& front() { 
        TYPUS_REQUIRES(!this->empty());
        return *begin_; 
    }

    /**
     * \brief Access the first element of the vector.
     *
     * \pre The vector is not empty.
     */
    inline const T& front() const { 
        TYPUS_REQUIRES(!this->empty());
        return *begin_; 
    }

    /**
     * \brief Access the last element of the vector.
     *
     * \pre The vector is not empty.
     */
    inline T& back() { 
        TYPUS_REQUIRES(!this->empty());
        return *(end_-1); 
    }

    /**
     * \brief Access the last element of the vector.
     *
     * \pre The vector is not empty.
     */
    inline const T& back() const { 
        TYPUS_REQUIRES(!this->empty());
        return *(end_-1); 
    }

    /**
     * \brief Append element to vector
     *
     * \pre The vector contains N elements
     * \post The vector contains N + 1 elements.
     */
    inline void push_back(const T& value) {
        if (end_ < capacity_) {
            new (end_) T(value);
            ++end_;
            return;
        }
        this->push_back_slow_path(value);
    }

    /**
     * \brief Append new item through in-place construction.
     * \pre The vector contains N elements
     * \post The vector contains N + 1 elements. 
     */
    template <typename ...As>
    inline void emplace_back(As &&...args) {
        if (end_ < capacity_) {
            new(end_) T(std::forward<As>(args)...);
            ++end_;
            return;
        }
        this->emplace_back_slow_path(std::forward<As>(args)...);
    }

    /**
     * \brief Remove the last element from the vector.
     *
     * \pre The element contains at least one element.
     * \post The last element has been removed from the vector.
     */
    inline void pop_back() {
        TYPUS_REQUIRES(!this->empty());
        (end_-1)->~T();
        --end_;
    }

    /**
     * \brief Resize the vector to size n
     *
     * Excessive elements are destroyed, missing elements are 
     * default-constructed.
     *
     * \post The vector contains N elements.
     */
    void resize(std::size_t n);

    /**
     * \brief Remove all elements from the vector
     *
     * This does not reset the vector to use the small storage area.
     *
     * \post The vector is empty.
     */
    void clear();

    /**
     * \brief Append elements from the iterator range begin, end to the 
     *     vector.
     *
     * \pre The vector contains N elements
     * \post The vector contains N + (begin - end) elements. The elements
     *    are append after existing elements in order.
     */
    template <typename I>
    inline void append(I begin, I end) {
        // use optimized version in case the number of elements in the
        // range begin, end is known.
        using category = 
            typename std::iterator_traits<I>::iterator_category;
        this->append(begin, end, category{});
    }
private:
    // the number of elements to append is known
    template <typename I>
    void append(I begin, I end, std::random_access_iterator_tag) {
        this->grow_to_hold_at_least(this->size() + end - begin);
        std::uninitialized_copy(begin, end, end_);
        end_ += end - begin;
    }

    // the number of elements is unknown.
    template <typename I>
    void append(I begin, I end, std::input_iterator_tag) {
        for (; begin != end; ++begin) {
            this->push_back(*begin);
        }
    }
    void grow_to_hold_at_least(std::size_t n);
    void push_back_slow_path(const T&value);
    template <typename ...As>
    void emplace_back_slow_path(As &&...args);
};


template <typename T>
void small_vector<T>::clear() {
    destroy_range(begin_, end_);
    end_ = begin_;
}


template <typename T>
void small_vector<T>::grow_to_hold_at_least(std::size_t n) {
    std::size_t new_capacity = detail::next_power_of_two(n);
    TYPUS_REQUIRES(new_capacity > this->capacity());
    bool free_required = !this->is_small();
    T* new_begin = static_cast<T*>(malloc(sizeof(T) * new_capacity));
    uninitialized_move_range(begin_, end_, new_begin);
    end_ = new_begin + (end_ - begin_);
    capacity_ = new_begin + new_capacity;
    if (free_required) {
        free(begin_);
    }
    begin_ = new_begin;
}

template <typename T>
void small_vector<T>::push_back_slow_path(const T&value) {
    TYPUS_REQUIRES(end_ == capacity_);
    this->grow_to_hold_at_least(this->size() + 1u);
    new(end_) T(value);
    ++end_;
}

template <typename T>
template <typename ...As>
void small_vector<T>::emplace_back_slow_path(As &&...args) {
    TYPUS_REQUIRES(end_ == capacity_);
    this->grow_to_hold_at_least(this->size() + 1u);
    new(end_) T(std::forward<As>(args)...);
    ++end_;
}

template <typename T>
void small_vector<T>::resize(std::size_t n) {
    std::size_t size = this->size();
    if (n < size) {
        T* new_end = begin_ + n;
        destroy_range(new_end, end_);
        end_ = new_end;
        return;
    }
    if (size > this->capacity()) {
        this->grow_to_hold_at_least(n);
    }
    T* new_end = begin_ + n;
    std::uninitialized_fill(end_, new_end, T{});
    end_ = new_end;
}

template <typename T, std::size_t S>
class small_vector_n : public small_vector<T> {
private:
    // additional storage to hold the remaining elements. The first
    // element is already stored in small_vector<T>.
    detail::small_storage<T, S> remainder_;
public:
    small_vector_n(): small_vector<T>(S) { }

    template <typename I>
    small_vector_n(I begin, I end): small_vector<T>(begin, end, S) {
    }
    small_vector_n(const small_vector_n<T, S> &vec):
        small_vector_n(vec.begin(), vec.end()) {
    }
    small_vector_n(small_vector_n<T, S> &&vec):
        small_vector<T>(S) {
        this->operator=(std::move(vec));
    }

    small_vector_n<T, S> &operator=(const small_vector_n<T, S> &rhs);
    small_vector_n<T, S> &operator=(small_vector_n<T, S> &&rhs);
};

template <typename T, std::size_t S>
small_vector_n<T, S> &small_vector_n<T, S>::operator=(const small_vector_n<T, S> &rhs) {
    this->clear();
    this->append(rhs.begin_, rhs.end_);
    return *this;
}


template <typename T, std::size_t S>
small_vector_n<T, S> &small_vector_n<T, S>::operator=(small_vector_n<T, S> &&rhs) {
    // delete excessive elements
    bool lhs_small = this->is_small();
    bool rhs_small = rhs.is_small();
    if (rhs_small) {
        if (!lhs_small) {
            destroy_range(this->begin_, this->end_);
            free(this->begin_);
            this->begin_ = &reinterpret_cast<T&>(this->head_);
            this->end_ = this->begin_;
            this->capacity_ = this->begin_ + S;
        }
        T *dst = this->begin_;
        T *src = rhs.begin_;
        for (; dst != this->end_; ++dst, ++src) {
            dst->operator=(std::move(*src)); 
        }
        this->end_ = uninitialized_move_range(src, rhs.end_, dst);
        rhs.begin_ = rhs.end_ = rhs.capacity_ = nullptr;
        return *this;
    }
    destroy_range(this->begin_, this->end_);
    if (!lhs_small) {
        free(this->begin_);
    }
    this->begin_ = rhs.begin_;
    this->end_ = rhs.end_;
    this->capacity_ = rhs.capacity_;
    rhs.begin_ = rhs.end_ = rhs.capacity_ = nullptr;
    return *this;
}

}
#endif // TYPUS_SMALL_VECTOR_HH

