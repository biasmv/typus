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

TEST(Result, value_constructed_result_contains_value) {
    result<std::string>  one(std::string("one, two, three"));

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

namespace typus {
template <>
struct error_traits<Error> {
    constexpr static Error ok_value() { return Error::Ok; }
    static bool is_ok(const Error& error) {
        return error == Error::Ok;
    }
};

}

TEST(Result, with_custom_error_type) {
    result<int, Error> r1(1);
    ASSERT_TRUE(r1.ok());
    ASSERT_EQ(1, r1.value());
    result<int, Error> r2 = result<int, Error>::fail(Error::NoSuchFile);
    ASSERT_FALSE(r2.ok());
    ASSERT_EQ(Error::NoSuchFile, r2.error());
}

