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

#ifndef TYPUS_RESULT_HH
#define TYPUS_RESULT_HH

#include <memory>
#include <cassert>
#include <type_traits>

#include "assert.hh"


namespace typus {

namespace detail {

// Tag type for failed result construction. 
struct failed_tag_t {};



// holder class for the actual result. This class is required, because we want 
// to have separate implementations for trivially destructible value types and 
// types that require the destructor to be invoked.
template <typename T, typename E, 
          bool =std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value>
class result_storage {
protected:
    template <typename T2, typename E2>
    friend void construct(result_storage<T2, E2, false> &&, T2 &&, E2 &&, bool);

    result_storage(): value_(), ok_(true) {
    }
    
    result_storage(const result_storage &rhs): ok_(rhs.ok_) {
        construct(rhs.value_, rhs.error_, ok_);
    }

    result_storage(result_storage &&rhs): ok_(rhs.ok_) {
        construct(std::move(rhs.value_), std::move(rhs.error_), ok_);
    }

    result_storage(const T &rhs): 
        value_(rhs), ok_(true) {
    }

    result_storage(typename std::remove_reference<T>::type &&rhs): 
        value_(std::move(rhs)), ok_(true) {
    }

    result_storage(failed_tag_t, E error): error_(error), ok_(false) {
    }

    result_storage &operator=(const result_storage& rhs) {
        this->destroy();
        this->ok_ = rhs.ok_;
        construct(rhs.value_, rhs.error_, this->ok_);
        return *this;
    }

    result_storage &operator=(result_storage&& rhs) {
        this->destroy();
        this->ok_ = rhs.ok_;
        construct(std::move(rhs.value_), std::move(rhs.error_), this->ok_);
        return *this;
    }

    ~result_storage() {
        this->destroy();
    }
protected:
    union {
        T value_;
        E error_;
    };
    bool ok_;
private:
    template <typename T2, typename E2>
    inline void construct(T2 && v, E2 &&e, bool ok) {
        if (ok) {
            ::new (std::addressof(this->value_)) T(std::forward<T2>(v));
        } else {
            ::new (std::addressof(this->error_)) E(std::forward<E2>(e));
        }
    }
    inline void destroy() {
        if (this->ok_) {
            this->value_.~T();
        } else {
            this->error_.~E();
        }
    }
};

template <typename T, typename E>
class result_storage<T, E, true> {
protected:

    result_storage(): value_(), ok_(true) {
    }
    
    result_storage(const result_storage &rhs): ok_(rhs.ok_) {
        construct(rhs.value_, rhs.error_, ok_);
    }

    result_storage(result_storage &&rhs): ok_(rhs.ok_) {
        construct(std::move(rhs.value_), std::move(rhs.error_), ok_);
    }

    result_storage(const T &rhs): 
        value_(rhs), ok_(true) {
    }

    result_storage(typename std::remove_reference<T>::type &&rhs): 
        value_(std::move(rhs)), ok_(true) {
    }

    result_storage(failed_tag_t, E error): error_(error), ok_(false) {
    }

    result_storage &operator=(const result_storage& rhs) {
        this->ok_ = rhs.ok_;
        construct(rhs.value_, rhs.error_, this->ok_);
        return *this;
    }

    result_storage &operator=(result_storage&& rhs) {
        this->ok_ = rhs.ok_;
        construct(std::move(rhs.value_), std::move(rhs.error_), this->ok_);
        return *this;
    }

    ~result_storage() = default;
protected:
    union {
        T value_;
        E error_;
    };
    bool ok_;
private:
    template <typename T2, typename E2>
    inline void construct(T2 && v, E2 &&e, bool ok) {
        if (ok) {
            ::new (std::addressof(this->value_)) T(std::forward<T2>(v));
        } else {
            ::new (std::addressof(this->error_)) E(std::forward<E2>(e));
        }
    }
};

} // namespace detail


/**
 * \brief class holding a value of type T or an error of type E.
 *
 * Typical uses of this class include as a return type for a function/method 
 * that can fail. The result acts as a replacement for exception-based error 
 * handling.
 *
 * This class can be used as a replacement for std::optional (using a boolean 
 * error type).
 */
template <typename T, typename E=bool>
class result : public detail::result_storage<T, E> {
public:
    typedef T value_type;
    typedef E error_type;

public:
    /**
     * \brief create a result containing a default-constructed value.
     */
    result(): detail::result_storage<T, E>() {}

    /**
     * \brief copy-construct a result.
     */
    result(const result<T, E>& rhs): detail::result_storage<T, E>(rhs) { }

    /**
     * \brief construct a new result holding a value by copy-constructor.
     */
    result(const value_type &rhs): detail::result_storage<T, E>(rhs) {}

    /**
     * \brief construct a new result holding a value through move construction.
     */
    result(value_type &&rhs): detail::result_storage<T, E>(std::move(rhs)) {}


    result(result<T, E> &&rhs): detail::result_storage<T, E>(std::move(rhs)) {
    }

    template <typename T2, typename E2>
    friend class result;

    /**
     * Allow for implicit conversion from one failed result type to another. 
     * Only supported if error types are the same. aborts if the value is
     * ok. 
     */
    template<typename U>
    result(const result<U, E> &other): 
        detail::result_storage<T,E>(detail::failed_tag_t{}, other.error_) {
        assert(!other.ok());
    }

    /**
     * \brief construct a failed result using the provided error value
     */
    static result<T, E> fail(E error) {
        return result<T,E>(detail::failed_tag_t{}, error);
    }
    /**
     * \brief construct a failed result using the default fail value.
     */
    static result<T, E> fail() {
        return result<T,E>(detail::failed_tag_t{}, E{});
                           
    }

    result<T, E> &operator=(const result<T, E> &rhs) = default;
    result<T, E> &operator=(result<T, E> &&rhs) = default;

    /**
     * \brief The error state of the result.
     */
    E error() const { 
        TYPUS_REQUIRES(!this->ok());
        return this->error_;
    }

    /**
     * \brief whether the result contains a valid value.
     */
    bool ok() const {
        return this->ok_;
    }

    /**
     * \brief whether the result contains a valid value.
     *
     * Identical to calling \ref ok.
     */
    explicit operator bool() const {
        return this->ok();
    }


    /**
     * \brief access the value in-place.
     *
     * Aborts if the result does not hold a valid value.
     */
    const T& value() const { 
        TYPUS_REQUIRES(this->ok());
        return this->value_; 
    }
    /**
     * \brief access the value in-place.
     *
     * Aborts if the result does not hold a valid value.
     */
    T& value() { 
        TYPUS_REQUIRES(this->ok());
        return this->value_; 
    }

    /**
     * \brief extract the value out of the result.
     */
    T && extract() {
        TYPUS_REQUIRES(this->ok());
        return std::move(this->value_);
    }

    /**
     * \brief Returns the value contained in the result, or in case of
     *       error, the value provided as the method argument.
     */
    const T &value_or(const T& error_value) const {
        if (this->ok()) {
            return this->value_;
        }
        return error_value;
    }

    /**
     * \brief Returns the value contained in the result, or in case of
     *       error, the value provided as the method argument.
     */
    T &value_or(T& error_value) {
        if (this->ok()) {
            return this->value_;
        }
        return error_value;
    }

    /**
     * \brief Apply the function \p func to the contained value, or return
     *     this object in case result holds an error.
     */
    template <typename F>
    result<T, E> and_then(F && func) {
        // allows functions that return result<T, E> as well as T
        return this->ok() ?  func(this->value_) : *this;
    }

private:
    result(detail::failed_tag_t, const error_type &e): 
        detail::result_storage<T, E>(detail::failed_tag_t{}, e) {
    }
};

/**
 * macro for Rust-style function return value checking. Note that is is
 * not standard C++ and is only supported in clang and gcc, since it relies
 * on statement expressions. So if you want to be portable, don't use it.
 *
 * Typical usage:
 * \code
 * result<std::string> may_fail() { ... }
 *
 * result<std::string> foo() {
 *    std::string value  = TRY(may_fail());
 *    return do_stuff(value); *  
 * }
 * \endcode
 */
#define TRY(expr) ({ \
         auto v = expr; \
         if (!v.ok()) { \
             return v; \
         }; \
         v.extract(); \
    })


} // namespace typus

#endif // TYPUS_RESULT_HH

