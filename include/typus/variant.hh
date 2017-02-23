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

#ifndef TYPUS_VARIANT_HH
#define TYPUS_VARIANT_HH

#include <type_traits>
#include <tuple>

namespace typus {

namespace details {

template <std::size_t I, typename S, typename T, typename ...Ts>
struct index_of : 
    public std::conditional<std::is_same<S, T>::value,
                            typename std::integral_constant<std::size_t, I>::type,
                            index_of<I+1, S, Ts...>>::type {
};

template <typename T>
struct NOT_A_VALID_ALTERNATIVE {};


// used for calling the correct destructor
struct destroy_stored_object {
    template <typename T>
    void operator()(T & object) {
        object.~T();
    }
};

// use for copy construction
template <typename S>
struct copy_stored_object {
    S &storage;

    template <typename T>
    void operator()(const T & object) {
        new (&storage) T { object };
        object.~T();
    }
};

}

/**
 * \brief A data type holding one of multiple alternatives.
 */
template <typename ...Ts>
class variant {
private:
    // helper template for retrieving the index of the provided type
    template <typename T>
    using index_of = 
        typename details::index_of<0, T, Ts..., 
                                   details::NOT_A_VALID_ALTERNATIVE<T>>;
public:
    // helper template for retrieving the type of the Nth alternative
    template <std::size_t N>
    using type_of = typename std::tuple_element<N, std::tuple<Ts...>>::type;

public:
    variant(): index_(0) {
        new (&storage_) type_of<0>{};
    }

    template <typename T>
    explicit variant(const T& rhs): index_(index_of<T>()) {
        new (&storage_) T{rhs};
    }

    explicit variant(const variant &rhs): index_(rhs.index_) {
        details::copy_stored_object<storage> in_place_copier{ storage_ };
        rhs.visit(in_place_copier);
    }

    ~variant() {
        this->destroy_stored_object();
    }

    /**
     * \brief The index of the selected alternative 
     */
    std::size_t index() const { return index_; }
    
    template <std::size_t N>
    type_of<N> & get() { 
        return reinterpret_cast<type_of<N>&>(storage_); 
    }

    template <std::size_t N>
    const type_of<N> & get() const { 
        return reinterpret_cast<const type_of<N>&>(storage_); 
    }

    template <typename T>
    T & get() {
        return reinterpret_cast<T&>(storage_);
    }
    template <typename T>
    const T & get() const {
        return reinterpret_cast<const T&>(storage_);
    }

    template <typename T>
    variant &operator=(const T& rhs) {
        this->destroy_stored_object();
        index_ = index_of<T>();
        new (&storage_) T{rhs};
        return *this;
    }

    variant &operator=(const variant& rhs) {
        this->destroy_stored_object();
        index_ = rhs.index();
        details::copy_stored_object<storage> in_place_copier{ storage_ };
        rhs.visit(in_place_copier);
        return *this;
    }

    template <typename F>
    void visit(F& func) {
        this->visit_stored_object<F, 0, Ts...>(func);
    }

    template <typename F>
    void visit(F& func) const {
        this->visit_stored_object<F, 0, Ts...>(func);
    }
private:
    void destroy_stored_object() {
        details::destroy_stored_object destroyer{};
        this->visit(destroyer);
    }
    template <typename F, std::size_t I>
    void visit_stored_object(F &func) {}

    template <typename F, std::size_t I, typename X, typename ...Xs>
    void visit_stored_object(F &func) {
        if (I == index_) {
            func(reinterpret_cast<X&>(storage_));
        }
        this->visit_stored_object<F, I+1,Xs...>(func);
    }

    template <typename F, std::size_t I>
    void visit_stored_object(F &func) const {}

    template <typename F, std::size_t I, typename X, typename ...Xs>
    void visit_stored_object(F &func) const {
        if (I == index_) {
            func(reinterpret_cast<const X&>(storage_));
        }
        this->visit_stored_object<F, I+1,Xs...>(func);
    }
private:
    using storage = typename std::aligned_union<0u, Ts...>::type;

    storage storage_;
    std::size_t index_;
};

}
#endif // TYPUS_VARIANT_HH

