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
#include <typus/variant.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(Variant, default_construction) {
    variant<std::string, int, char> var;
    ASSERT_EQ(0u, var.index());
    ASSERT_EQ(std::string(""), var.get<0>());
    ASSERT_EQ(std::string(""), var.get<std::string>());
}


struct A {
    ~A() { value += 1; }

    int &value;
};

TEST(Variant, correct_destructor_gets_called) {
    int value = 0;
    A object { value };
    {
        variant<std::string, A> var1{ object };
    }
    ASSERT_EQ(1, value);
}


TEST(Variant, copy_construction) {
    using V = variant<std::string, int, char>;
    V a('b');
    V b(a);
    ASSERT_EQ(a.index(), b.index());
    ASSERT_EQ(a.get<char>(), b.get<char>());
}

TEST(Variant, alternative_construction) {
    {
        variant<std::string, int, char> var{ std::string{"one"} };
        ASSERT_EQ(0u, var.index());
        ASSERT_EQ(std::string("one"), var.get<0>());
    }
    
    {
        variant<std::string, int, char> var{ 3 };
        ASSERT_EQ(1u, var.index());
        ASSERT_EQ(3, var.get<1>());
    }
    
    {
        variant<std::string, int, char> var{ 'b' };
        ASSERT_EQ(2u, var.index());
        ASSERT_EQ('b', var.get<2>());
    }
}

TEST(Variant, assignment) {
    variant<std::string, int, char> var{ std::string{"one"} };

    var = std::string("two");
    ASSERT_EQ(0u, var.index());
    ASSERT_EQ(std::string("two"), var.get<0>());
}
