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
#include <typus/result.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(Result, default_constructed_result_contains_default_constructed_value) {
    result<std::string>  one;

    ASSERT_TRUE(one.ok());
    ASSERT_TRUE(one);
    ASSERT_EQ("", one.value());
}

struct no_copy_ctor {
    no_copy_ctor() = default;
    no_copy_ctor(const no_copy_ctor&) = delete;
    no_copy_ctor(no_copy_ctor&&) = default;
    no_copy_ctor& operator=(no_copy_ctor&&) = default;
    no_copy_ctor& operator=(const no_copy_ctor&) = delete;
    int val = 3;
};

TEST(Result, move_constructed_result_contains_moved_value) {
    result<no_copy_ctor> one{no_copy_ctor{}};
    ASSERT_TRUE(one.ok());
    ASSERT_TRUE(one);
    ASSERT_EQ(3, one.value().val);

}


TEST(Result, move_assignment) {
    result<no_copy_ctor> one = result<no_copy_ctor>::fail();
    ASSERT_FALSE(one.ok());
    one = std::move(no_copy_ctor{});
    ASSERT_EQ(3, one.value().val);
    ASSERT_TRUE(one.ok());
}

TEST(Result, copy_assignment) {

}

TEST(Result, value_constructed_result_contains_value) {
    std::string value{"one, two, three"};
    result<std::string>  one{value};

    ASSERT_TRUE(one.ok());
    ASSERT_TRUE(one);
    ASSERT_EQ("one, two, three", one.value());
}


struct NonTrivialDestructor {
    bool &invoked;

    ~NonTrivialDestructor() { invoked = true; }
};

TEST(Result, destructor_gets_invoked_for_non_trivially_destructible_types) {
    bool invoked = false;
    NonTrivialDestructor value { invoked };
    {
        result<NonTrivialDestructor> r(value);
    }
    ASSERT_TRUE(invoked);
}

struct TrivialDestructor {
    bool &invoked;

    ~TrivialDestructor() { invoked = true; }
};

namespace std {

// technically a lie, but useful for testing
template <>
struct is_trivially_destructible<TrivialDestructor> : public std::true_type { 
};

}

TEST(Result, destructor_does_not_get_invoked_for_trivially_destructible_types) {
    bool invoked = false;
    TrivialDestructor value { invoked };
    {
        result<TrivialDestructor> r(value);
    }
    ASSERT_FALSE(invoked);
}


TEST(Result, failed_result_conversion) {
    result<std::string> one = result<std::string>::fail();
    result<int> two = one;
    ASSERT_FALSE(two.ok());
    ASSERT_FALSE(one.ok());
}

enum class Error {
    Ok, NoSuchFile, NoHardDrive
};
template <>
struct error_traits<Error> {
    constexpr static Error ok_value() { return Error::Ok; }
    static bool is_ok(const Error& error) {
        return error == Error::Ok;
    }
};

TEST(Result, with_custom_error_type) {
    result<int, Error> r1(1);
    ASSERT_TRUE(r1.ok());
    ASSERT_EQ(1, r1.value());
    result<int, Error> r2 = result<int, Error>::fail(Error::NoSuchFile);
    ASSERT_FALSE(r2.ok());
    ASSERT_EQ(Error::NoSuchFile, r2.error());
}


TEST(Result, value_or) {
    result<std::string> one = result<std::string>::fail();
    ASSERT_EQ(std::string{"bad value"}, one.value_or("bad value"));
    one = result<std::string>{"good value"};
    ASSERT_EQ(std::string{"good value"}, one.value_or("bad value"));
}

TEST(Result, and_then) {
    result<std::string> one = result<std::string>::fail();
    ASSERT_FALSE(one.and_then([](const std::string&) { 
        return result<std::string>{"ok"};
    }).ok());
    one = result<std::string>{"good value"};
    ASSERT_EQ(std::string{"good value"}, one.value_or("bad value"));
    ASSERT_EQ("ok", one.and_then([](const std::string&) { 
        return result<std::string>{"ok"};
    }).value());
    ASSERT_EQ("ok", one.and_then([](const std::string&) { 
        return std::string{"ok"};
    }).value());
}

