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
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <type_traits>


namespace typus {

template <typename E>
struct error_traits {
    /**
     * Whether the result is OK.
     */
    static bool is_ok(const E& value);

    /**
     * Error value to be used to indicate that result is OK, e.g. contains a 
     * constructed value of type T.
     */
    constexpr static E ok_value();

    /**
     * May be provided for an error type as the default error statu, so 
     * \code result<T,E>::fail() \endcode can be called without arguments. For 
     * types where there is no clear default failed value, this can be omitted.
     */
    constexpr static E default_fail_value();
};

/**
 * Default error traits for boolean errors.
 */
template <>
struct error_traits<bool> {
    static bool is_ok(const bool& error) {
        return !error;
    }
    constexpr static bool ok_value() { return false; }
    constexpr static bool default_fail_value() { return true; }
};


namespace detail {

// Tag type for failed result construction. 
struct failed_tag_t {};

// holder class for the actual result. This class is required, because we want 
// to have separate implementations for trivially destructible value types and 
// types that require the destructor to be invoked.
template <typename T, typename E, 
          bool =std::is_trivially_destructible<T>::value>
class result_storage {
public:
    using value_type = T;
    using error_type = E;
protected:
    result_storage(): value_(), error_(error_traits<E>::ok_value()) {
    }
    
    result_storage(const result_storage &rhs): error_(rhs.error_) {
        if (error_traits<E>::is_ok(error_)) {
            // in-place new
            ::new (std::addressof(value_)) value_type(rhs.value_);
        }
    }

    result_storage(const result_storage &&rhs): error_(rhs.error_) {
        if (error_traits<E>::is_ok(error_)) {
            // in-place new
            ::new (std::addressof(value_)) T(std::move(rhs.value_));
        }
    }

    result_storage(const value_type &rhs): 
        value_(rhs), error_(error_traits<E>::ok_value()) {
    }

    result_storage(const value_type &&rhs): 
        value_(std::move(rhs)), error_(error_traits<E>::ok_value()) {
    }

    result_storage(failed_tag_t, E error): nul_state_('\0'), error_(error) {
    }

    ~result_storage() {
        if (error_traits<E>::is_ok(error_)) {
            value_.~T();
        }
    }
    union {
        char nul_state_;
        T value_;
    };
    E error_;
};

template <typename T, typename E>
class result_storage<T, E, true> {
public:
    using value_type = T;
    using error_type = E;
protected:
    result_storage(): value_(), error_(error_traits<E>::ok_value()) {
    }
    
    result_storage(const result_storage &rhs): error_(rhs.error_) {
        if (error_traits<E>::is_ok(error_)) {
            // in-place new
            ::new (std::addressof(value_)) value_type(rhs.value_);
        }
    }

    result_storage(const result_storage &&rhs): error_(rhs.error_) {
        if (error_traits<E>::is_ok(error_)) {
            // in-place new
            ::new (std::addressof(value_)) value_type(std::move(rhs.value_));
        }
    }

    result_storage(const value_type &rhs): 
        value_(rhs), error_(error_traits<E>::ok_value()) {
    }

    result_storage(const value_type &&rhs): 
        value_(std::move(rhs)), error_(error_traits<E>::ok_value()) {
    }

    result_storage(failed_tag_t, E error): nul_state_('\0'), error_(error) {
    }

    // nothing to be done, value_type is trivially destructible
    ~result_storage() { }
    union {
        char nul_state_;
        T value_;
    };
    E error_;
};

} // namespace detail

/**
 * \brief class holding a value of type T or an error of type E.
 *
 * Typical uses of this class include as a return type for a function/method 
 * that can fail. The result acts as a replacement for exception-based error 
 * handling.
 */
template <typename T, typename E=bool>
class result : public detail::result_storage<T, E> {
public:
    typedef T value_type;
    typedef E error_type;

    /**
     * \brief construct a failed result using the provided error value
     */
    static result<T, E> fail(E error) {
        return result<T,E>(detail::failed_tag_t{}, error);
    }
    /**
     * \brief construct a failed result using the default fail value.
     *
     * This is probably only useful when using a boolean error type as 
     * for all others there is not meaningful default error value.
     */
    static result<T, E> fail() {
        return result<T,E>(detail::failed_tag_t{}, 
                           error_traits<E>::default_fail_value());
    }

    /**
     * \brief whether the result contains a valid value.
     */
    bool ok() const {
        return error_traits<E>::is_ok(this->error_);
    }

    /**
     * \brief whether the result contains a valid value.
     *
     * Identical to calling \ref ok.
     */
    operator bool() const {
        return this->ok();
    }


    /**
     * \brief access the value in-place.
     *
     * Aborts if the result does not hold a valid value.
     */
    const T& value() const { 
        assert(this->ok());
        return this->value_; 
    }
    /**
     * \brief access the value in-place.
     *
     * Aborts if the result does not hold a valid value.
     */
    T& value() { 
        assert(this->ok());
        return this->value_; 
    }

    /**
     * \brief extract the value out of the result.
     */
    T && extract() {
        assert(this->ok());
        return std::move(this->value_);
    }
public:
    /**
     * \brief create a result containing a default-constructed value.
     */
    result() = default;
    /**
     * \brief copy-construct a result.
     */
    result(const result& rhs) = default;
    /**
     * \brief construct a new result holding a value by copy-constructor.
     */
    result(const  value_type &rhs): detail::result_storage<T, E>(rhs) {}
    /**
     * \brief construct a new result holding a value through move construction.
     */
    result(value_type &&rhs): detail::result_storage<T, E>(std::move(rhs)) {}

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
     * \brief The error state of the result.
     */
    E error() const { return this->error_; }
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
 *    return do_stuff(value);
 *  
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

