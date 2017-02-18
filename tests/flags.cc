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
#include <typus/flags.hh>

#include <gtest/gtest.h>

using namespace typus;

enum Enum {
    EnumOne = 0x01,
    EnumTwo = 0x02
};

enum class EnumClass {
    One = 0x01,
    Two = 0x02
};

TEST(Flags, construction) {
    flags<Enum> flags_a;
    ASSERT_EQ(0, flags_a.bits());
    flags<EnumClass> flags_b;
    ASSERT_EQ(0, flags_b.bits());

    ASSERT_EQ(0x01, flags<Enum>(EnumOne).bits());
    ASSERT_EQ(0x02, flags<EnumClass>(EnumClass::Two).bits());

    ASSERT_EQ(0x03, flags<Enum>({EnumOne, EnumTwo}).bits());
    ASSERT_EQ(0x03, flags<Enum>(EnumOne, EnumTwo).bits());
}

TEST(Flags, constexpr_support) {
    constexpr flags<Enum> flags_a(EnumOne, EnumTwo);
    static_assert(flags_a.bits() == 3u, "Uups");

}

TEST(Flags, bit_wise_ops) {
    flags<Enum> flags_a;
    flags_a |= EnumOne;
    ASSERT_EQ(0x01, flags_a.bits());
    flags<Enum> flags_b = flags<Enum>(EnumOne) | EnumTwo;
    ASSERT_EQ(0x03, flags_b.bits());
    ASSERT_TRUE(flags_b.is_set(EnumOne));
    ASSERT_TRUE(flags_b.is_set(EnumTwo));
    flags_b.clear_all();
    ASSERT_FALSE(flags_b.is_set(EnumOne));
    ASSERT_FALSE(flags_b.is_set(EnumTwo));
    ASSERT_FALSE(flags_a.is_set(EnumTwo));
}

